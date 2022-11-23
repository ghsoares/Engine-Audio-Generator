#ifndef ENGINE_AUDIO_RECORDER_H
#define ENGINE_AUDIO_RECORDER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Dictionary.hpp>
#include <Ref.hpp>
#include <AudioStreamSample.hpp>
#include "engine_config.h"

namespace godot {

class EngineAudioRecorder : public Reference {
	GODOT_CLASS(EngineAudioRecorder, Reference)
private:
	Ref<EngineConfig> engine_config;
	Ref<AudioStreamSample> recording;

	float min_rpm;
	float top_rpm;
	float duration_per_sample;
	int preheat_cycles;
	int sample_count;
	bool include_audio_header;

public:
	static void _register_methods();

	void set_engine_configuration(Ref<EngineConfig> p_config) {
		engine_config = p_config;
		if (p_config.is_valid()) {
			p_config->mark_dirty();
		}
	}
	Ref<EngineConfig> get_engine_configuration() const {return engine_config;}

	void set_min_rpm(float p_rpm) {min_rpm = p_rpm;}
	float get_min_rpm() const {return min_rpm;}

	void set_top_rpm(float p_rpm) {top_rpm = p_rpm;}
	float get_top_rpm() const {return top_rpm;}

	void set_duration_per_sample(float p_duration) {duration_per_sample = p_duration;}
	float get_duration_per_sample() const {return duration_per_sample;}

	void set_preheat_cycles(int p_cycles) {preheat_cycles = p_cycles;}
	int get_preheat_cycles() const {return preheat_cycles;}

	void set_sample_count(int p_sample_count) {sample_count = p_sample_count;}
	int get_sample_count() const {return sample_count;}

	void set_include_audio_header(bool p_include) {include_audio_header = p_include;}
	bool get_include_audio_header() const {return include_audio_header;}

	void record();
	Ref<AudioStreamSample> get_recording() const {return recording;}

	void _init();

	EngineAudioRecorder();
	~EngineAudioRecorder();
};

}

#endif // ENGINE_AUDIO_RECORDER_H