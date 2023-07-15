#ifndef ENGINE_AUDIO_GENERATOR_H
#define ENGINE_AUDIO_GENERATOR_H

#include "engine_simulator.h"

#include <Godot.hpp>
#include <Reference.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>

namespace godot {

// The main simulator
class EngineAudioGenerator : public Reference {
	GODOT_CLASS(EngineAudioGenerator, Reference);
public:
	Ref<EngineSimulator> simulator;
	Ref<AudioStreamGenerator> stream;
	Ref<AudioStreamGeneratorPlayback> playback;
public:
	void set_simulator(Ref<EngineSimulator> p_simulator) {simulator = p_simulator;}
	Ref<EngineSimulator> get_simulator() const {return simulator;}

	void set_stream(Ref<AudioStreamGenerator> p_stream) {stream = p_stream;}
	Ref<AudioStreamGenerator> get_stream() const {return stream;}

	void set_playback(Ref<AudioStreamGeneratorPlayback> p_playback) {playback = p_playback;}
	Ref<AudioStreamGeneratorPlayback> get_playback() const {return playback;}

	void push_frames(int p_frames);

	void _init();

	EngineAudioGenerator();
	~EngineAudioGenerator();

	static void _register_methods();
};

}

#endif // ENGINE_AUDIO_GENERATOR_H