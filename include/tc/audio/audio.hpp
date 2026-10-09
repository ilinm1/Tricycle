#pragma once

#include <mutex>
#include <unordered_map>
#include <portaudio.h>
#include "audio_file.hpp"
#include "stream.hpp"

#define TCA_DEFAULT_SAMPLE_RATE 44100
#define TCA_MIX_TIMEOUT 32
#define TCA_BUFFERING_RATE 8

namespace Tc::Audio
{
	bool ConfigureIo(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE);

	//file methods

	AudioFile LoadFile(std::filesystem::path path, bool loadAll = true);
	void PlayFile(AudioFile* filePtr, bool repeat = false);
	void SeekFile(AudioFile* filePtr, unsigned int sample);
	void SeekFileSeconds(AudioFile* filePtr, float seconds);
	void StopFile(AudioFile* filePtr, bool close = true);
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

	//init, update, shutdown

	void Initialize(bool stereo = true, bool input = false, unsigned int sampleRate = TCA_DEFAULT_SAMPLE_RATE);
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
