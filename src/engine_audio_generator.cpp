#include "engine_audio_generator.h"
#include <GodotGlobal.hpp>
#include <iostream>

using namespace godot;

bool EngineAudioGenerator::validate_config() {
	ERR_FAIL_COND_V(!playback.is_valid(), false);
	ERR_FAIL_COND_V(!stream.is_valid(), false);
	ERR_FAIL_COND_V(!engine_config.is_valid(), false);
	ERR_FAIL_COND_V(!engine_config->is_engine_valid(), false);

	return true;
}

void EngineAudioGenerator::fill_buffer(int p_max_frames) {
	ERR_FAIL_COND(!validate_config());
	
	int frames = (int)playback->get_frames_available();

	frames = frames < p_max_frames ? frames : p_max_frames;

	if (frames <= 0) return;

	ERR_FAIL_COND(!playback->can_push_buffer(frames));

	PoolVector2Array buffer;
	buffer.resize(frames);
	PoolVector2Array::Write buf = buffer.write();

	engine_config->fill_buffer((float *)buf.ptr(), frames, 2);

	float dt = 1.f / engine_config->get_sample_rate();
	float compensation_lerp = Math::clamp(dt / 1.f, 0.f, 1.f);
	float max_mix = 0.f;

	for (int i = 0; i < frames; i++) {
		float l = buf[i].x;
		float r = buf[i].y;
		float mix = std::abs(l) > std::abs(r) ? l : r;
		max_mix = mix > max_mix ? mix : max_mix;
	}
	
	for (int i = 0; i < frames; i++) {
		float desired_vol = 1.f;
		if (max_mix > 1.f) {
			desired_vol = 1.f / max_mix;
		}
		compensation_volume += Math::clamp(desired_vol - compensation_volume, -compensation_lerp, compensation_lerp);

		buf[i] *= compensation_volume;
	}

	waveguides_dampened = engine_config->get_waveguides_dampened();

	playback->push_buffer(buffer);
}

void EngineAudioGenerator::_init() {
	
}

void EngineAudioGenerator::_register_methods() {
	register_property<EngineAudioGenerator, Ref<AudioStreamGenerator>>(
		"stream", 
		&EngineAudioGenerator::set_stream,
		&EngineAudioGenerator::get_stream,
		Ref<AudioStreamGenerator>()
	);
	register_property<EngineAudioGenerator, Ref<AudioStreamGeneratorPlayback>>(
		"playback", 
		&EngineAudioGenerator::set_playback,
		&EngineAudioGenerator::get_playback,
		Ref<AudioStreamGeneratorPlayback>()
	);
	register_property<EngineAudioGenerator, Ref<EngineConfig>>(
		"engine_configuration", 
		&EngineAudioGenerator::set_engine_configuration,
		&EngineAudioGenerator::get_engine_configuration,
		Ref<EngineConfig>()
	);
	
	register_method("fill_buffer", &EngineAudioGenerator::fill_buffer);
	register_method("get_waveguides_dampened", &EngineAudioGenerator::get_waveguides_dampened);
}

EngineAudioGenerator::EngineAudioGenerator() {
	this->waveguides_dampened = false;
	this->compensation_volume = 1.f;

	this->stream = Ref<AudioStreamGenerator>();
	this->playback = Ref<AudioStreamGeneratorPlayback>();
	this->engine_config = Ref<EngineConfig>();
}

EngineAudioGenerator::~EngineAudioGenerator() {
	
}