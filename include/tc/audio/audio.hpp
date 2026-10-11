#pragma once

#include <mutex>
#include <unordered_map>
#include <portaudio.h>
#include "audio_file.hpp"
#include "stream.hpp"

#define TCA_DEFAULT_SAMPLE_RATE 44100
#define TCA_MIX_TIMEOUT 32
#define TCA_BUFFERING_RATE 8
#define TCA_MAX_SPEEDUP 16

namespace Tc::Audio
{
	//general methods

	//(re)configures the IO stream with the given parameters
	//returns true if stream was opened and started succesfully
	bool ConfigureIo(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE);

	//downsamples 'inSamples' samples from 'in' and writes the result to 'out'; 'in' and 'out' may be the same buffer
	//returns the amount of newly generated samples
	unsigned int Downsample(float* in, float* out, unsigned int inRate, unsigned int outRate, unsigned int inSamples);

	//upsamples 'inSamples' samples from 'in' and writes the result to 'out'
	//returns the amount of newly generated samples
	unsigned int Upsample(float* in, float* out, unsigned int inRate, unsigned int outRate, unsigned int inSamples);

	//file methods

	//loads an audio file from 'path' and returns the associated 'AudioFile' object
	//if 'buffered' is set the entirety of the file will be decoded and stored in memory; otherwise it will be decoded piece by piece from the disk (streamed)
	//second option reduces the amount of memory used by the audio, as well as the execution time of this method but requires running the 'UpdateLoop' method to continue loading the audio
	//also streamed files cannot be copied so in order to play the same file with different parameters you will need to call 'LoadFile' again
	AudioFile LoadFile(std::filesystem::path path, bool buffered = true);

	//starts playing the file, doesn't block the caller thread
	//if 'repeat' is set then the file will be played indefinitely until 'StopFile' is called
	//if the file is already playing this method does nothing, you may use 'SeekFile' to start playing the same file again or create a copy of the file IF IT'S BUFFERED
	void PlayFile(AudioFile* filePtr, bool repeat = false);

	//see also: 'SeekFileSeconds'
	void SeekFile(AudioFile* filePtr, unsigned int sample);

	void SeekFileSeconds(AudioFile* filePtr, float seconds);

	//if 'free' is set then the file's resources will be freed, and it won't be possible to play it again; set it if you're sure you won't need it anymore to save some resources
	//this will also make all copies of the file unusable so keep that in mind
	void StopFile(AudioFile* filePtr, bool free = false);

	void SetVolume(AudioFile* filePtr, float volume);

	void SetVolume(AudioFile* filePtr, float left, float right);

	//1 is the original speed, max speedup relative to the output stream is defined by 'TCA_MAX_SPEEDUP'
	void SetSpeed(AudioFile* filePtr, float speed);

	//stream methods

	//adds the specified stream to the left channel if 'left' is set or stereo audio is disabled, or to the right channel otherwise
	void AddStream(Stream* streamPtr, bool left);

	//removes the stream from the left channel if 'left' is set, or from the right channel otherwise
	void RemoveStream(Stream* streamPtr, bool left);

	//removes the stream from both channels
	void RemoveStream(Stream* streamPtr);

	void RemoveAllStreams();

	//you can do the same by directly changing the stream's 'Volume', this exists just to make the header prettier
	void SetVolume(Stream* streamPtr, float volume);

	//1 is the original speed, max speedup relative to the output stream is defined by 'TCA_MAX_SPEEDUP'
	void SetSpeed(Stream* streamPtr, float speed);

	//init, update, shutdown

	void Initialize(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE);

	//blocks the current thread
	void UpdateLoop();

	void Shutdown();

	//globals

	inline const std::unordered_map<std::string, AudioFileFormat> AudioFileFormats = { { ".ogg", AudioFileFormat::Ogg } };

	inline std::vector<Stream*> LeftChannelStreams; //if output is mono then this will be the only available channel
	inline std::vector<Stream*> RightChannelStreams;
	inline std::vector<AudioFile*> AudioFiles; //all currently played files
	inline std::recursive_mutex AudioFileOpLock;

	inline PaStream* IoStreamPtr;
	inline unsigned int IoSampleRate;
	inline bool StereoEnabled = true;
	inline bool InputEnabled;
}
