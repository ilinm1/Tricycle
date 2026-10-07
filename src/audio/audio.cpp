#include <vector>
#include <stdexcept>
#include <filesystem>
#include <portaudio.h>
#include <stb_vorbis.c>
#include "tc/misc/misc.hpp"
#include "tc/audio/audio.hpp"
#include "tc/audio/audio_file.hpp"
#include "tc/audio/stream.hpp"

#ifdef TC_USE_SIMD //x86 architecture with AVX support assumed
#include <immintrin.h>
#endif

namespace Tca = Tc::Audio;

bool CheckPaErrors(PaError error)
{
    if (error != PaErrorCode::paNoError)
    {
        Tc::Log(std::format("PortAudio error {} - '{}'", error, Pa_GetErrorText(error)));
        return false;
    }

    return true;
}

//callback

//downsamples 'inSamples' samples from 'in' and writes the result to 'out'; 'in' and 'out' may be the same buffer
//returns the amount of newly generated samples
unsigned int Downsample(float* in, float* out, unsigned int inRate, unsigned int outRate, unsigned int inSamples)
{
    if (inRate < outRate)
        throw std::runtime_error("Old sample rate must be higher than new sample rate when downsampling.");

    float ratio = static_cast<float>(inRate) / outRate;
    int minSamples = static_cast<int>(ratio);
    int period = static_cast<int>(1.0f / fmodf(ratio, 1.0f));

    unsigned int inIndex = 0, outIndex = 0;
    while (inIndex < inSamples)
    {
        out[outIndex] = in[inIndex];
        inIndex += minSamples + (period == 0 || outIndex % period ? 0 : 1);
        outIndex++;
    }

    return outIndex;
}

//upsamples 'inSamples' samples from 'in' and writes the result to 'out'
//returns the amount of newly generated samples
unsigned int Upsample(float* in, float* out, unsigned int inRate, unsigned int outRate, unsigned int inSamples)
{
    if (inRate > outRate)
        throw std::runtime_error("Old sample rate must be lower than new sample rate when upsampling.");

    float ratio = static_cast<float>(inRate) / outRate;
    int minSamples = static_cast<int>(ratio);
    int period = static_cast<int>(1.0f / fmodf(ratio, 1.0f));

    unsigned int inIndex = 0, outIndex = 0;
    while (inIndex < inSamples)
    {
        float a = in[inIndex];
        float b = in[inIndex + 1];

        int count = minSamples + (period == 0 || inIndex % period ? 0 : 1);
        for (unsigned int i = 0; i < count; i++, outIndex++)
        {
            out[outIndex] = a + (b - a) * static_cast<float>(i) / count;
        }

        inIndex++;
    }

    return outIndex;
}

void MixStreams_NoSimd(float* data, unsigned int samples, std::vector<Tca::Stream*>& streams)
{
    for (Tca::Stream* streamPtr : streams)
    {
        unsigned int noSamples = 0;
        while (samples)
        {
            float temp[1024]; //buffer sizes were chosen arbitrarily
            float upsample[128];

            unsigned int toRead = std::min(samples, 128u);
            unsigned int ratio = (streamPtr->SampleRate + Tca::IoSampleRate - 1) / Tca::IoSampleRate;
            toRead = streamPtr->SampleRate > Tca::IoSampleRate ? toRead * ratio : toRead / ratio;
            unsigned int generated = streamPtr->Read(temp, toRead);

            if (!generated)
            {
                if (noSamples++ == TCA_STREAM_READ_TIMEOUT)
                    break;
            }

            if (streamPtr->SampleRate > Tca::IoSampleRate)
            {
                generated = Downsample(temp, temp, streamPtr->SampleRate, Tca::IoSampleRate, generated);
            }
            if (streamPtr->SampleRate < Tca::IoSampleRate)
            {
                generated = Upsample(temp, upsample, streamPtr->SampleRate, Tca::IoSampleRate, generated);
            }

            for (unsigned int i = 0; i < generated; i++)
            {
                data[i] = 0;
                data[i] += (streamPtr->SampleRate < Tca::IoSampleRate ? upsample[i] : temp[i]) * streamPtr->Volume;
            }

            data += generated;
            samples -= generated;
        }
    }
}

#ifdef TC_USE_SIMD
//'samples' should be a multiple of 8 to avoid reading past the end of 'data' which may cause a segfault under some conditions
void MixStreams_Simd(float* data, unsigned int samples, std::vector<Tca::Stream*>& streams)
{
    float temp[128]; //128 is an arbitrary number, that should be enough to store samples before downsampling
    for (; samples > 0; samples -= 8)
    {
        __m256 dataVec = _mm256_setzero_ps();
        for (Tca::Stream* streamPtr : streams)
        {
            __m256 streamVec, volumeVec;

            unsigned int ratio = (streamPtr->SampleRate + Tca::IoSampleRate - 1) / Tca::IoSampleRate;
            unsigned int toRead = streamPtr->SampleRate > Tca::IoSampleRate ? 8 * ratio : 8 / ratio;
            unsigned int read = streamPtr->Read(temp, toRead);

            if (!read)
                continue;

            if (streamPtr->SampleRate > Tca::IoSampleRate)
            {
                Downsample(temp, temp, streamPtr->SampleRate, Tca::IoSampleRate, read);
            }
            else if (streamPtr->SampleRate < Tca::IoSampleRate)
            {
                Upsample(temp, data, streamPtr->SampleRate, Tca::IoSampleRate, read);
            }

            streamVec = _mm256_loadu_ps(streamPtr->SampleRate < Tca::IoSampleRate ? data : temp);
            volumeVec = _mm256_broadcast_ss(&streamPtr->Volume); //filling vector with the same volume value
            dataVec = _mm256_add_ps(dataVec, _mm256_mul_ps(streamVec, volumeVec)); //adding multiplied stream samples to data vector
        }

        _mm256_storeu_ps(data, dataVec);
        data += 8;
    }
}
#endif

//mixes samples from the 'streams' and 'data' buffer and writes the result back to 'data'
void MixStreams(float* data, unsigned int samples, std::vector<Tca::Stream*>& streams)
{
#ifdef TC_USE_SIMD
    unsigned int forSimd = samples / 8 * 8;
    unsigned int leftover = samples % 8;
    MixStreams_Simd(data, forSimd, streams);
    if (leftover)
        MixStreams_NoSimd(data + forSimd, leftover, streams);
#else
    MixStreams_NoSimd(data, samples, streams);
#endif
}

int IoCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    static bool outputZeroed = true;
    bool anyStreams = false;

    if (Tca::LeftChannelStreams.size())
    {
        anyStreams = true;
        MixStreams(*reinterpret_cast<float**>(output), frameCount, Tca::LeftChannelStreams);
    }

    if (Tca::StereoEnabled && Tca::RightChannelStreams.size())
    {
        anyStreams = true;
        MixStreams(*(reinterpret_cast<float**>(output) + 1), frameCount, Tca::RightChannelStreams);
    }

    if (!anyStreams)
    {
        if (!outputZeroed)
        {
            std::memset(*reinterpret_cast<float**>(output), 0, frameCount * sizeof(float));
            if (Tca::StereoEnabled)
                std::memset(*(reinterpret_cast<float**>(output) + 1), 0, frameCount * sizeof(float));
            outputZeroed = true;
        }
    }
    else
    {
        outputZeroed = false;
    }

    return PaStreamCallbackResult::paContinue;
}

//file methods

void BufferFile_Ogg(Tca::AudioFile* filePtr, unsigned int samples)
{
    if (!filePtr->AllLoaded)
    {
        static float leftChannel[TCA_DEFAULT_STREAM_BUF_SZ], rightChannel[TCA_DEFAULT_STREAM_BUF_SZ];
        float* channels[2];

        bool alloc = samples > TCA_DEFAULT_STREAM_BUF_SZ; //only allocate on heap if we need to load a lot of data at once, right now
        if (alloc)
        {
            channels[0] = new float[samples];
            channels[1] = new float[samples];
        }
        else
        {
            channels[0] = leftChannel;
            channels[1] = rightChannel;
        }

        unsigned int toRead = std::min(samples, filePtr->LeftChannelStream.CanWrite()); //only checking how much left channel can write but right will be the same if they're played at the same speed
        unsigned int fromFile = stb_vorbis_get_samples_float(static_cast<stb_vorbis*>(filePtr->Handle), 2, channels, toRead);
        unsigned int toWrite = std::min(toRead, fromFile);
        filePtr->LeftChannelStream.Write(channels[0], toWrite);
        if (Tca::StereoEnabled && filePtr->Stereo)
            filePtr->RightChannelStream.Write(channels[1], toWrite);

        if (alloc)
        {
            delete[] channels[0];
            delete[] channels[1];
        }
    }

    if (!filePtr->LeftChannelStream.CanRead())
    {
        if (filePtr->Repeat)
        {
            Tca::SeekFile(filePtr, 0);
        }
        else
        {
            Tca::StopFile(filePtr);
        }
    }
}

//loads the next piece of audio data from disk into the specified file's streams and handles playback end
void BufferFile(Tca::AudioFile* filePtr, unsigned int samples)
{
    switch (filePtr->Format)
    {
        case Tca::AudioFileFormat::Ogg:
            BufferFile_Ogg(filePtr, samples);
            break;
        default:
            throw std::runtime_error("Tried buffering a file with an unknown format.");
    }
}

Tca::AudioFile LoadFile_Ogg(std::filesystem::path path, bool loadAll)
{
    int error;
    stb_vorbis* file = stb_vorbis_open_filename(path.string().c_str(), &error, nullptr);
    if (!file)
        throw std::runtime_error(std::format("stb_vorbis couldn't load the file - error code {}.", error));

    unsigned int samples = stb_vorbis_stream_length_in_samples(file);
    Tca::AudioFile result(
        path,
        Tca::AudioFileFormat::Ogg,
        file->sample_rate,
        samples,
        file->channels > 1,
        loadAll ? samples + 1 : TCA_DEFAULT_STREAM_BUF_SZ,
        file
    );

    BufferFile(&result, loadAll ? result.TotalSamples : TCA_DEFAULT_STREAM_BUF_SZ);
    result.AllLoaded = true;

    return result;
}

//loads an audio file from 'path' and returns the associated 'AudioFile' object
//if 'loadAll' is set the entirety of the file will be decoded and stored in memory; otherwise it will be decoded piece by piece
//second option reduces the amount of memory used by the audio, as well as the execution time of this method but requires running the 'UpdateLoop' method to continue loading the audio
Tca::AudioFile Tca::LoadFile(std::filesystem::path path, bool loadAll)
{
    std::string ext = path.extension().string();

    if (!AudioFileFormats.contains(ext))
        throw std::runtime_error(std::format("'{}' is not a supported audio file format.", ext));

    switch (AudioFileFormats.at(ext))
    {
        case AudioFileFormat::Ogg:
            return LoadFile_Ogg(path, loadAll);
    }
}

//don't forget to set file's 'Repeat' to play it indefinitely
void Tca::PlayFile(AudioFile* filePtr)
{
    if (filePtr->Playing)
        return;

    SeekFile(filePtr, 0);
    AddStream(&filePtr->LeftChannelStream, true);
    AddStream(&filePtr->RightChannelStream, false);
    AudioFiles.push_back(filePtr);
    filePtr->Playing = true;
}

void Tca::SeekFile(AudioFile* filePtr, unsigned int sample)
{
    if (filePtr->AllLoaded)
    {
        filePtr->LeftChannelStream.ReadPtr = filePtr->LeftChannelStream.Buffer;
        filePtr->RightChannelStream.ReadPtr = filePtr->RightChannelStream.Buffer;
    }

    switch (filePtr->Format)
    {
        case AudioFileFormat::Ogg:
            stb_vorbis_seek_frame(reinterpret_cast<stb_vorbis*>(filePtr->Handle), sample);
            break;
        default:
            throw std::runtime_error("Tried seeking a file with an unknown format.");
    }
}

void Tca::SeekFileSeconds(AudioFile* filePtr, float seconds)
{
    SeekFile(filePtr, filePtr->SampleRate * seconds);
}

//if 'close' is set then file will be closed, and it won't be possible to play it again; set it if you're sure you won't need it anymore to save some resources
void Tca::StopFile(AudioFile* filePtr, bool close)
{
    if (!filePtr->Playing)
        return;

    RemoveStream(&filePtr->LeftChannelStream, true);
    RemoveStream(&filePtr->RightChannelStream, false);
    AudioFiles.erase(std::find(AudioFiles.begin(), AudioFiles.end(), filePtr));
    filePtr->Playing = false;
}

void Tca::SetVolume(AudioFile* filePtr, float volume)
{
    filePtr->LeftChannelStream.Volume = volume;
}

void Tca::SetVolume(AudioFile* filePtr, float left, float right)
{
    if (!filePtr->Stereo)
        throw std::runtime_error("Tried setting volume for two channels on a non-stereo audio file.");

    filePtr->LeftChannelStream.Volume = left;
    filePtr->RightChannelStream.Volume = right;
}

void Tca::SetSpeed(AudioFile* filePtr, float speed)
{
    SetSpeed(&filePtr->LeftChannelStream, speed);
    if (StereoEnabled)
        SetSpeed(&filePtr->RightChannelStream, speed);
}

//stream methods

//adds the specified stream to the left channel if 'left' is set or stereo audio is disabled, or to the right channel otherwise
void Tca::AddStream(Stream* streamPtr, bool left)
{
    if (!StereoEnabled || left)
    {
        LeftChannelStreams.push_back(streamPtr);
    }
    else
    {
        RightChannelStreams.push_back(streamPtr);
    }
}

//removes the stream from both channels
void Tca::RemoveStream(Stream* streamPtr)
{
    RemoveStream(streamPtr, true);
    if (StereoEnabled)
        RemoveStream(streamPtr, false);
}

//removes the stream from the left channel if 'left' is set, or from the right channel otherwise
void Tca::RemoveStream(Stream* streamPtr, bool left)
{
    if (left)
    {
        LeftChannelStreams.erase(std::find(LeftChannelStreams.begin(), LeftChannelStreams.end(), streamPtr));
    }
    else
    {
        RightChannelStreams.erase(std::find(RightChannelStreams.begin(), RightChannelStreams.end(), streamPtr));
    }
}

void Tca::RemoveAllStreams()
{
    LeftChannelStreams.clear();
    RightChannelStreams.clear();
}

//you can do the same by directly changing the stream's 'Volume', this exists just to make the header prettier
void Tca::SetVolume(Stream* streamPtr, float volume)
{
    streamPtr->Volume = volume;
}

void Tca::SetSpeed(Stream* streamPtr, float speed)
{
    streamPtr->SampleRate = static_cast<unsigned int>(speed * streamPtr->OriginalSampleRate);
}

//(re)configures the IO stream with the given parameters
//returns true if stream was opened and started succesfully
bool Tca::ConfigureIo(bool stereo, bool input, unsigned int sampleRate, bool destroyStreams)
{
    if (IoStreamPtr && Pa_IsStreamActive(IoStreamPtr))
        CheckPaErrors(Pa_AbortStream(Tca::IoStreamPtr));

    if (CheckPaErrors(Pa_OpenDefaultStream(&IoStreamPtr, input ? 1 : 0, stereo ? 2 : 1, paFloat32 | paNonInterleaved, sampleRate, paFramesPerBufferUnspecified, &IoCallback, nullptr)) &&
        CheckPaErrors(Pa_StartStream(IoStreamPtr)))
    {
        IoSampleRate = sampleRate;
        StereoEnabled = stereo;
        InputEnabled = input;

        if (!StereoEnabled && stereo)
        {
            for (AudioFile* filePtr : AudioFiles)
            {
                filePtr->RightChannelStream.ReadPtr = filePtr->LeftChannelStream.ReadPtr;
                AddStream(&filePtr->RightChannelStream, false);
            }
        }

        if (!stereo)
            RightChannelStreams.clear();

        return true;
    }

    return false;
}

//init, update

void Tca::Initialize(bool stereo, bool input, unsigned int sampleRate)
{
    if (!CheckPaErrors(Pa_Initialize()))
        throw std::runtime_error("Failed to initialize PortAudio.");

    Log(std::format("PortAudio version: {}", Pa_GetVersionInfo()->versionText));
    if (!ConfigureIo(stereo, input, sampleRate))
        throw std::runtime_error("Failed to configure audio I/O stream.");
}

//blocks the current thread
void Tca::UpdateLoop()
{
    while (true)
    {
        for (AudioFile* filePtr : AudioFiles)
        {
            BufferFile(filePtr, TCA_DEFAULT_STREAM_BUF_SZ);
        }
    }
}

//should be called after audio is no longer in use
void Tca::Shutdown()
{
    CheckPaErrors(Pa_CloseStream(IoStreamPtr));
    CheckPaErrors(Pa_Terminate());
}
