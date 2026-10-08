#pragma once

//in samples
#define TCA_DEFAULT_STREAM_BUF_SZ 4096

namespace Tc::Audio
{
	/*
	represents a single-channel, PCM audio stream (to be clear - this isn't a wrapper around PortAudio's streams)
	each stream has a circular buffer, one thread writes data to it and then the audio thread running the 'IoCallback' reads from it to actually play sounds
	so to play any sounds you have to create a stream, write data to it and call 'AddStream' to add it to the left/right channel
	but you don't have to use this unless you want to manually generate audio data; for playing audio from a file use the 'AudioFile' class
	*/
	struct Stream
	{
		float Volume; //can be changed at any point
		unsigned int OriginalSampleRate; //mainly used for setting speed using the 'SetSpeed' method
		unsigned int SampleRate; //can be changed at any point

		float* Buffer;
		float* WritePtr; //points at the next sample which will be written
		float* ReadPtr; //points at the next sample which will be read
		unsigned int BufferSize;

		Stream(unsigned int sampleRate, unsigned int bufferSize = TCA_DEFAULT_STREAM_BUF_SZ, float* buffer = nullptr);
		~Stream();

		unsigned int CanWrite();
		unsigned int CanRead();
		unsigned int Write(float* data, unsigned int size);
		unsigned int Read(float* data, unsigned int size);
		void WriteAll(float* data, unsigned int size);
		void ReadAll(float* data, unsigned int size);
	};
}
