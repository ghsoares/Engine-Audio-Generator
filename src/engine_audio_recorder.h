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
	Ref<AudioStreamSample> crankshaft_recording;
	Ref<AudioStreamSample> ignition_recording;
	Ref<AudioStreamSample> exhaust_recording;

	float min_rpm;
	float top_rpm;
	float duration_per_sample;
	float preheat_time;
	float fade_time;
	int sample_count;
	int padding_frames;
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

	void set_preheat_time(float p_time) {preheat_time = p_time;}
	float get_preheat_time() const {return preheat_time;}

	void set_fade_time(float p_time) {fade_time = p_time;}
	float get_fade_time() const {return fade_time;}

	void set_sample_count(int p_sample_count) {sample_count = p_sample_count;}
	int get_sample_count() const {return sample_count;}

	void set_padding_frames(int p_frames) {padding_frames = p_frames;}
	int get_padding_frames() const {return padding_frames;}

	void set_include_audio_header(bool p_include) {include_audio_header = p_include;}
	bool get_include_audio_header() const {return include_audio_header;}

	void record();
	Ref<AudioStreamSample> get_crankshaft_recording() const {return crankshaft_recording;}
	Ref<AudioStreamSample> get_ignition_recording() const {return ignition_recording;}
	Ref<AudioStreamSample> get_exhaust_recording() const {return exhaust_recording;}

	void _init();

	EngineAudioRecorder();
	~EngineAudioRecorder();
};

}

#endif // ENGINE_AUDIO_RECORDER_H