#pragma once

#include <filesystem>
#include "stream.hpp"

namespace Tc::Audio
{
	//todo: implement more formats (mp3 atleast https://github.com/lieff/minimp3)
	enum AudioFileFormat
	{
		Ogg
	};

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
		bool AllLoaded; //whether the file is already completely loaded or not
		bool Repeat; //if set file will continue playing indefinitely until 'StopFile' is called
		bool Playing; //whether the file is currently playing - setting this yourself won't change anything (and will actually break stuff so this should be encapsulated (as well as bunch of other data in other types) - todo)
	
		AudioFile(std::filesystem::path path, AudioFileFormat format, unsigned int sampleRate, unsigned int totalSamples, bool stereo, unsigned int streamBufferSize, void* handle);
	};
}
