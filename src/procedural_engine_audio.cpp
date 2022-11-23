#include "procedural_engine_audio.h"
#include <Math.hpp>

using namespace godot;

void ProceduralEngineAudioGenerator::update_buffer() {
	ERR_FAIL_COND(!stream.is_valid());
	ERR_FAIL_COND(!playback.is_valid());

	int frames = (int)playback->get_frames_available();
	PoolVector2Array frames_buffer;
	frames_buffer.resize(frames);
	PoolVector2Array::Write frames_write = frames_buffer.write();

	float inc = (frequency / (float)rate);

	for (int i = 0; i < frames; i++) {
		float s = (float)Math::sin(phase * Math_TAU);
		frames_write[i] = Vector2(s, s);
		phase = Math::fmod(phase + inc, 1.0f);
	}

	playback->push_buffer(frames_buffer);
}

void ProceduralEngineAudioGenerator::_init() {
	rate = 22050;
	frequency = 440.0;
	phase = 0.0;
	stream = Ref<AudioStreamGenerator>();
	playback = Ref<AudioStreamGenerator>();
}

void ProceduralEngineAudioGenerator::_register_methods() {
	register_method("update_buffer", &ProceduralEngineAudioGenerator::update_buffer);

	register_property<ProceduralEngineAudioGenerator, Ref<AudioStreamGenerator>>(
		"stream", 
		&ProceduralEngineAudioGenerator::set_stream,
		&ProceduralEngineAudioGenerator::get_stream, 
		Ref<AudioStreamGenerator>()
	);
	register_property<ProceduralEngineAudioGenerator, Ref<AudioStreamGeneratorPlayback>>(
		"playback",
		&ProceduralEngineAudioGenerator::set_playback,
		&ProceduralEngineAudioGenerator::get_playback,
		Ref<AudioStreamGeneratorPlayback>()
	);
	
	register_property<ProceduralEngineAudioGenerator, float>(
		"frequency", 
		&ProceduralEngineAudioGenerator::set_frequency,
		&ProceduralEngineAudioGenerator::get_frequency,
		22050
	);
	register_property<ProceduralEngineAudioGenerator, int>(
		"rate", 
		&ProceduralEngineAudioGenerator::set_rate,
		&ProceduralEngineAudioGenerator::get_rate,
		22050
	);
}

ProceduralEngineAudioGenerator::ProceduralEngineAudioGenerator() {}

ProceduralEngineAudioGenerator::~ProceduralEngineAudioGenerator() {}