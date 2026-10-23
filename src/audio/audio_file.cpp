#include <stb_vorbis.h>
#include "tc/audio/audio.hpp"
#include "tc/audio/audio_file.hpp"

namespace Tca = Tc::Audio;

Tca::AudioFile::AudioFile() :
	Format(Tca::AudioFileFormat::Invalid),
	SampleRate(0),
	TotalSamples(0),
	Stereo(false),
	LeftChannelStream(Tca::Stream(0, 0, nullptr)),
	RightChannelStream(Tca::Stream(0, 0, nullptr)),
	Handle(nullptr),
	Buffered(true),
	Repeat(false),
	Playing(false)
{}

//copy constructor
Tca::AudioFile::AudioFile(const AudioFile& other) :
	Path(other.Path),
	Format(other.Format),
	SampleRate(other.SampleRate),
	TotalSamples(other.TotalSamples),
	Stereo(other.Stereo),
	LeftChannelStream(Tca::Stream(other.LeftChannelStream.OriginalSampleRate, other.LeftChannelStream.BufferSize, other.LeftChannelStream.Buffer)),
	RightChannelStream(Tca::Stream(other.RightChannelStream.OriginalSampleRate, other.RightChannelStream.BufferSize, other.RightChannelStream.Buffer)),
	Handle(other.Handle),
	Buffered(true),
	Repeat(false),
	Playing(false)
{
	if (!other.Buffered)
		throw std::runtime_error("It's illegal to create copies of streamed audio files.");
}

//move constructor
Tca::AudioFile::AudioFile(AudioFile&& other) noexcept :
	Path(other.Path),
	Format(other.Format),
	SampleRate(other.SampleRate),
	TotalSamples(other.TotalSamples),
	Stereo(other.Stereo),
	LeftChannelStream(Tca::Stream(other.LeftChannelStream.OriginalSampleRate, other.LeftChannelStream.BufferSize, other.LeftChannelStream.Buffer)),
	RightChannelStream(Tca::Stream(other.RightChannelStream.OriginalSampleRate, other.RightChannelStream.BufferSize, other.RightChannelStream.Buffer)),
	Handle(other.Handle),
	Buffered(other.Buffered),
	Repeat(other.Repeat),
	Playing(other.Playing)
{
	other.Handle = nullptr;
	other.LeftChannelStream.Buffer = nullptr;
	other.RightChannelStream.Buffer = nullptr;
}

//if 'singleBuffer' is set both left and right channel streams will use the same buffer (for mono audio)
Tca::AudioFile::AudioFile(
	std::filesystem::path path,
	AudioFileFormat format,
	unsigned int sampleRate,
	unsigned int totalSamples,
	bool stereo,
	unsigned int streamBufferSize,
	void* handle) :
	Path(path),
	Format(format),
	SampleRate(sampleRate),
	TotalSamples(totalSamples), 
	Stereo(stereo),
	LeftChannelStream(Tca::Stream(sampleRate, streamBufferSize)),
	RightChannelStream(Tca::Stream(sampleRate, streamBufferSize, stereo ? nullptr : LeftChannelStream.Buffer)),
	Handle(handle),
	Buffered(false),
	Repeat(false),
	Playing(false) {}

Tca::AudioFile::~AudioFile()
{
	if (Playing)
		StopFile(this, false);

	if (!Handle)
		return;

	switch (Format)
	{
		case AudioFileFormat::Ogg:
			stb_vorbis_close(reinterpret_cast<stb_vorbis*>(Handle));
			break;
	}
}

//copy assignment
Tca::AudioFile& Tca::AudioFile::operator=(const AudioFile& other)
{
	if (!other.Buffered)
		throw std::runtime_error("It's illegal to create copies of streamed audio files.");

	std::memcpy(this, &other, sizeof(AudioFile));

	Repeat = Playing = false;
	LeftChannelStream.ReadPtr = LeftChannelStream.Buffer;
	LeftChannelStream.WritePtr = LeftChannelStream.Buffer + 1;
	RightChannelStream.ReadPtr = RightChannelStream.Buffer;
	RightChannelStream.WritePtr = RightChannelStream.Buffer + 1;

	return *this;
}

//move assignment
Tca::AudioFile& Tca::AudioFile::operator=(AudioFile&& other) noexcept
{
	std::memcpy(this, &other, sizeof(AudioFile));

	other.Handle = nullptr;
	other.LeftChannelStream.Buffer = nullptr;
	other.RightChannelStream.Buffer = nullptr;

	return *this;
}
