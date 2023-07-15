#include "virtual_microphone.h"
#include <cmath>

void VirtualMicrophone::record_impulse(float p_impulse, float p_position) {
	// Calculate

	// Get distance in meters
	float dst = std::abs(p_position - this->position);

	// Get time in seconds based on speed of sound = 343
	float time = dst / 343;

	// Get attenuation
	float att = 1 / (1 + dst * dst);

	// Get samples offset
	size_t samples = (size_t)(time * sample_rate);

	// Add to audio buffer
	this->audio_write->add(sample * att, samples);
}

VirtualMicrophone::VirtualMicrophone() {
	this->audio_write = nullptr;
	this->sample_rate = 44100;
	this->position = 3;
}

VirtualMicrophone::~VirtualMicrophone() {
	
}