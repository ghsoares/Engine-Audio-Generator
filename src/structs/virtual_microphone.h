#ifndef STRUCTS_VIRTUAL_MICROPHONE_H
#define STRUCTS_VIRTUAL_MICROPHONE_H

#include "ring_buffer.h"

class VirtualMicrophone {
private:
	RingBuffer<float>::Cursor *audio_write;
	float sample_rate;
	float position;
public:
	void set_audio_write(RingBuffer<float>::Cursor *p_write) {this->audio_write = p_write;}

	void set_sample_rate(float p_rate) {this->sample_rate = p_rate;}
	float get_sample_rate() const {return this->sample_rate;}

	void set_position(float p_pos) {this->position = p_pos;}
	float get_position() const {return this->position;}

	void record_audio_sample(float p_sample, float p_position);

	VirtualMicrophone();
	~VirtualMicrophone();
};

#endif // STRUCTS_VIRTUAL_MICROPHONE_H