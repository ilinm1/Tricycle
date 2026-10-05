#pragma once

#include <filesystem>

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
		bool Playing; //whether the file is currently playing - setting this yourself won't change anything
	};
}
