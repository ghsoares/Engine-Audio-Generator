#include "engine/physics/motion_body.h"
#include "engine/engine_simulator.h"
#include "engine/engine_audio_generator.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	godot::register_class<godot::MotionBody>();
	godot::register_class<godot::EngineGear>();
	godot::register_class<godot::EngineStarter>();
	godot::register_class<godot::EngineSimulator>();
	godot::register_class<godot::EngineAudioGenerator>();
}