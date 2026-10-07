#include "tc/audio/audio_file.hpp"

namespace Tca = Tc::Audio;

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
	AllLoaded(false),
	Repeat(false),
	Playing(false) {}
