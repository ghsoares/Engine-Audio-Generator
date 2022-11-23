#ifndef ENGINE_AUDIO_GENERATOR_H
#define ENGINE_AUDIO_GENERATOR_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Ref.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include "engine_config.h"

namespace godot {

class EngineAudioGenerator : public Reference {
	GODOT_CLASS(EngineAudioGenerator, Reference)
private:
	bool waveguides_dampened;
	float compensation_volume;

	Ref<AudioStreamGenerator> stream;
	Ref<AudioStreamGeneratorPlayback> playback;
	Ref<EngineConfig> engine_config;

	bool validate_config();
public:
	static void _register_methods();

	void set_stream(Ref<AudioStreamGenerator> p_stream) {stream = p_stream;}
	Ref<AudioStreamGenerator> get_stream() {return stream;}

	void set_playback(Ref<AudioStreamGeneratorPlayback> p_playback) {playback = p_playback;}
	Ref<AudioStreamGeneratorPlayback> get_playback() {return playback;}

	void set_engine_configuration(Ref<EngineConfig> p_config) {
		engine_config = p_config;
		if (p_config.is_valid()) {
			p_config->mark_dirty();
		}
	}
	Ref<EngineConfig> get_engine_configuration() const {return engine_config;}

	bool get_waveguides_dampened() const {return waveguides_dampened;}

	void fill_buffer(int p_max_frames);

	void _init();

	EngineAudioGenerator();
	~EngineAudioGenerator();
};

}

#endif // ENGINE_AUDIO_GENERATOR_H