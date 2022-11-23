#include "engine_config.h"
#include "engine_audio_generator.h"
#include "engine_audio_recorder.h"
#include "procedural_engine_audio.h"
#include "engine_audio_player.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
	godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
	godot::Godot::nativescript_init(handle);

	godot::register_class<godot::EngineConfig>();
	godot::register_class<godot::EngineCylinderConfig>();
	godot::register_class<godot::EngineMufflerConfig>();
	godot::register_class<godot::EngineAudioGenerator>();
	godot::register_class<godot::EngineAudioRecorder>();
	godot::register_class<godot::ProceduralEngineAudioGenerator>();
	godot::register_class<godot::EngineAudioPlayer>();
}