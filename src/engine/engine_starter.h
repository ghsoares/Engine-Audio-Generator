#ifndef ENGINE_STARTER_H
#define ENGINE_STARTER_H

#include "engine_gear.h"
#include "../structs/virtual_microphone.h"

#include <Godot.hpp>
#include <Reference.hpp>

namespace godot {

class EngineStarter : public Reference {
	GODOT_CLASS(EngineStarter, Reference);
public:
	// The main gear that links to other gears
	Ref<EngineGear> main_gear;

	// Starter factor
	float starter;

	// Starter speed and acceleration
	float starter_rpm;
	float starter_acceleration;

	// Spring force and drag
	float spring_force;
	float spring_drag;

public:
	void step(float p_dt);
	void gears_transmission(Ref<EngineGear> &p_g0, Ref<EngineGear> &p_g1, float p_dt);

	void record_audio_sample(VirtualMicrophone &p_microphone);

	void _init();

	EngineStarter();
	~EngineStarter();

	static void _register_methods();
};

}

#endif // ENGINE_STARTER_H