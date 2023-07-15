#include "engine_audio_generator.h"

using namespace godot;

void EngineAudioGenerator::push_frames(int p_frames) {
	ERR_FAIL_COND(!simulator.is_valid());
	ERR_FAIL_COND(!stream.is_valid());
	ERR_FAIL_COND(!playback.is_valid());

	// A error
	ERR_FAIL_COND(!playback->can_push_buffer(p_frames));

	// The buffer array
	PoolVector2Array buf_arr;
	buf_arr.resize(p_frames);

	// Get the write buffer
	PoolVector2Array::Write buf_write = buf_arr.write();

	// Get the write pointer
	Vector2 *buf_ptr = buf_write.ptr();

	// Get the read buffer
	RingBuffer<float>::Cursor *audio_read = &simulator->audio_read;

	// For each frame
	for (int i = 0; i < p_frames; i++) {
		// Get the audio frame
		float audio = audio_read->get();
		audio_read->set(0);
		audio_read->move();

		// Write to buffer
		buf_ptr[i] = Vector2(audio, audio);
	}

	// Push buffer
	playback->push_buffer(buf_arr);
}

void EngineAudioGenerator::_init() {
	
}

EngineAudioGenerator::EngineAudioGenerator() {
	simulator = Ref<EngineSimulator>();
	stream = Ref<AudioStreamGenerator>();
	playback = Ref<AudioStreamGeneratorPlayback>();
}

EngineAudioGenerator::~EngineAudioGenerator() {
	
}

void EngineAudioGenerator::_register_methods() {
	register_property<EngineAudioGenerator, Ref<EngineSimulator>>(
		"simulator", 
		&EngineAudioGenerator::set_simulator,
		&EngineAudioGenerator::get_simulator,
		Ref<EngineSimulator>()
	);
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

	register_method("push_frames", &EngineAudioGenerator::push_frames);
}