#pragma once

#include <unordered_map>
#include <portaudio.h>
#include "audio_file.hpp"
#include "stream.hpp"

#define TCA_DEFAULT_SAMPLE_RATE 44100

namespace Tc::Audio
{
	bool ConfigureIo(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE, bool removeStreams = true);

	//file methods

	AudioFile LoadFile(std::filesystem::path path, bool loadAll);
	void PlayFile(AudioFile* filePtr);
	void StopFile(AudioFile* filePtr);
	void SetVolume(AudioFile* filePtr, float volume);
	void SetVolume(AudioFile* filePtr, float left, float right);
	void SetSpeed(AudioFile* filePtr, float speed);

	//stream methods

	void AddStream(Stream* streamPtr, bool left);
	void RemoveStream(Stream* streamPtr);
	void RemoveStream(Stream* streamPtr, bool left);
	void RemoveAllStreams();
	void SetVolume(Stream* streamPtr, float volume);
	void SetSpeed(Stream* streamPtr, float speed);

	//init, update

	void Initialize(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE);
	void UpdateLoop();

	//globals

	const std::unordered_map<std::string, AudioFileFormat> AudioFileFormats = { { ".ogg", AudioFileFormat::Ogg } };

	inline std::vector<Stream*> LeftChannelStreams;
	inline std::vector<Stream*> RightChannelStreams;
	inline std::vector<AudioFile*> AudioFiles;

	inline PaStream* IoStreamPtr;
	inline unsigned int IoSampleRate;
	inline bool StereoEnabled;
	inline bool InputEnabled;
}
