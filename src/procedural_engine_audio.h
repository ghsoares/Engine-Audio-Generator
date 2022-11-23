#ifndef PROCEDURAL_ENGINE_AUDIO_H
#define PROCEDURAL_ENGINE_AUDIO_H

#include <Godot.hpp>
#include <Ref.hpp>
#include <Reference.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>

namespace godot {

class ProceduralEngineAudioGenerator : public Reference {
	GODOT_CLASS(ProceduralEngineAudioGenerator, Reference)

private:
	Ref<AudioStreamGenerator> stream;
	Ref<AudioStreamGeneratorPlayback> playback;
	int rate;
	float frequency;

	float phase;

public:
	static void _register_methods();

	void set_stream(Ref<AudioStreamGenerator> p_stream) {stream = p_stream;}
	Ref<AudioStreamGenerator> get_stream() const {return stream;}

	void set_playback(Ref<AudioStreamGeneratorPlayback> p_playback) {playback = p_playback;}
	Ref<AudioStreamGeneratorPlayback> get_playback() const {return playback;}

	void set_rate(int p_rate) {rate = p_rate;}
	int get_rate() const {return rate;}

	void set_frequency(float p_frequency) {frequency = p_frequency;}
	float get_frequency() const {return frequency;}

	void update_buffer();
	
	void _init();

	ProceduralEngineAudioGenerator();
	~ProceduralEngineAudioGenerator();
};

}

#endif // PROCEDURAL_ENGINE_AUDIO_H