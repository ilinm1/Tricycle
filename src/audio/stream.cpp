#include <algorithm>
#include <stdexcept>
#include "tc/audio/stream.hpp"

namespace Tca = Tc::Audio;

//'Stream' uses circular buffer to exchange data between threads reading audio files/generating samples and the audio callback method
Tca::Stream::Stream(unsigned int sampleRate, unsigned int bufferSize, float* buffer) : Volume(1.0f), SampleRate(sampleRate), OriginalSampleRate(sampleRate), BufferSize(bufferSize)
{
	Buffer = buffer ? buffer : new float[bufferSize];
	ReadPtr = Buffer;
	WritePtr = Buffer + 1;
	*ReadPtr = 0;
}

Tca::Stream::~Stream()
{
	delete[] Buffer;
}

unsigned int Tca::Stream::CanWrite()
{
	if (WritePtr == ReadPtr)
		throw std::runtime_error("Stream's read & write pointers should not be equal.");

	return ReadPtr - WritePtr - 1 + (WritePtr > ReadPtr ? BufferSize : 0);
}

unsigned int Tca::Stream::CanRead()
{
	if (WritePtr == ReadPtr)
		throw std::runtime_error("Stream's read & write pointers should not be equal.");

	return WritePtr - ReadPtr - 1 + (ReadPtr > WritePtr ? BufferSize : 0);
}

//writes as much data as possible (no more than 'size' samples) without overwriting data which hasn't been read yet
//returns the amount of samples written
unsigned int Tca::Stream::Write(float* data, unsigned int size)
{
	size = std::min(size, CanWrite());
	WriteAll(data, size);
	return size;
}

//reads as much data as possible (no more than 'size' samples) without reading data which has already been read/isn't yet written
//doesn't modify read samples in any way (so no volume/sample rate adjustments)
//returns the amount of samples read
unsigned int Tca::Stream::Read(float* data, unsigned int size)
{
	size = std::min(size, CanRead());
	ReadAll(data, size);
	return size;
}

//writes 'size' samples without accounting for read pointer's position
void Tca::Stream::WriteAll(float* data, unsigned int size)
{
	if (size > BufferSize) //instead of overwriting the buffer 10 times, we can start reading from a later position and do it once
	{
		WritePtr += (WritePtr - Buffer + size - BufferSize) % BufferSize;
		data += size - BufferSize;
		size = BufferSize;
	}

	unsigned int maxSize = BufferSize - (WritePtr - Buffer); //how much we can write until the buffer's end
	std::memcpy(WritePtr, data, std::min(size, maxSize) * sizeof(float));

	if (size < maxSize) //all the data we wanted to write fits in the interval between the write pointer and the buffer's end so we can stop here
	{
		WritePtr += size;
		return;
	}
	
	WritePtr = Buffer; //we still have something to write so we do it from the buffer's start
	std::memcpy(WritePtr, data, size - maxSize * sizeof(float));
	WritePtr += size - maxSize;
}

//reads 'size' samples without accounting for write pointer's position
//doesn't modify read samples in any way (so no volume/sample rate adjustments)
void Tca::Stream::ReadAll(float* data, unsigned int size)
{
	unsigned int maxSize = BufferSize - (ReadPtr - Buffer); //how much we can read until the buffer's end
	std::memcpy(data, ReadPtr, std::min(size, maxSize) * sizeof(float));

	if (size < maxSize) //all the data we wanted to read was in the interval between the write pointer and the buffer's end so we can stop here
	{
		ReadPtr += size;
		return;
	}

	ReadPtr = Buffer; //we still have something to read so we do it from the buffer's start
	std::memcpy(data, ReadPtr, (std::min(size, BufferSize) - maxSize) * sizeof(float));

	if (size > BufferSize) //copy the read data instead of repeating all of the above steps
	{
		for (unsigned int i = 1; size > 0; i++, size -= BufferSize)
		{
			std::memcpy(data + BufferSize * i, data, std::min(size, BufferSize) * sizeof(float));
		}
	}
}
