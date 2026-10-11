#pragma once

#include <filesystem>
#include "stream.hpp"

namespace Tc::Audio
{
	//todo: implement more formats (mp3 https://github.com/lieff/minimp3)
	enum AudioFileFormat
	{
		Invalid,
		Ogg
	};

	//represents an audio file; can be either buffered or streamed
	//buffered files can be copied using 'MakeBufferedFileCopy' to play the same sound
	struct AudioFile
	{
		std::filesystem::path Path;
		AudioFileFormat Format;
		unsigned int SampleRate;
		unsigned int TotalSamples; //per each channel

		bool Stereo; //whether this audio file contains stereo audio
		Stream LeftChannelStream;
		Stream RightChannelStream;

		void* Handle; //currently used to store pointer to 'stb_vorbis', if more formats are added in the future this may hold something else
		bool Buffered; //if the file is buffered all of it's data is already loaded, otherwise it's loaded from the disk piece by piece (streamed)
		bool Repeat; //if set file will continue playing indefinitely until 'StopFile' is called
		bool Playing; //whether the file is currently playing - setting this yourself won't change anything (and will actually break stuff so this should be encapsulated (as well as bunch of other data in other types) - todo)

		AudioFile();
		AudioFile(const AudioFile& other);
		AudioFile(AudioFile&& other) noexcept;
		AudioFile(std::filesystem::path path, AudioFileFormat format, unsigned int sampleRate, unsigned int totalSamples, bool stereo, unsigned int streamBufferSize, void* handle);
		~AudioFile();
		AudioFile& operator=(const AudioFile& other);
		AudioFile& operator=(AudioFile&& other) noexcept;
	};
}
