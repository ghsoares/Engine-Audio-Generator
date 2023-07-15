#ifndef ENGINE_SIMULATOR_H
#define ENGINE_SIMULATOR_H

#include "engine_starter.h"
#include "../structs/virtual_microphone.h"

#include <Godot.hpp>
#include <Reference.hpp>

namespace godot {

// The main simulator
class EngineSimulator : public Reference {
	GODOT_CLASS(EngineSimulator, Reference);
public:
	Ref<EngineStarter> engine_starter;
	RingBuffer<float> audio_buffer;
	RingBuffer<float>::Cursor audio_write;
	RingBuffer<float>::Cursor audio_read;
	VirtualMicrophone microphone;

	float starter_factor;
	float time;

public:
	void simulate(float p_dt, int p_frames);
	void draw_simulation(const RID p_canvas_item);

	void set_starter_factor(float p_factor) {starter_factor = p_factor;}
	float get_starter_factor() const {return starter_factor;}

	void set_audio_buffer_size(int p_size) {audio_buffer.resize((size_t)p_size);}
	int get_audio_buffer_size() const {return (int)audio_buffer.get_size();}

	void set_audio_sample_rate(float p_rate) {microphone.set_sample_rate(p_rate);}
	float get_audio_sample_rate() const {return microphone.get_sample_rate();}

	void _init();

	EngineSimulator();
	~EngineSimulator();

	static void _register_methods();
};

}

#endif // ENGINE_SIMULATOR_H