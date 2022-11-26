#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include <Godot.hpp>
#include <Resource.hpp>
#include <Array.hpp>
#include "engine_parts.h"

namespace godot {

class EngineConfig;
class EngineCylinderConfig;
class EngineMufflerConfig;

class EngineConfig : public Resource {
	GODOT_CLASS(EngineConfig, Resource);
private:
	EngineMain *engine;
	bool engine_valid;
	LowPassFilter *dc_filter;
	bool engine_dirty;

	// Mix
	float rpm;
	float volume;
	float intake_volume;
	float exhaust_volume;
	float vibrations_volume;
	float dc_filter_frequency;
	bool waveguides_dampened;
	uint32_t sample_rate;

	// Engine params
	float vibrations_filter_frequency;
	float intake_noise_factor;
	float intake_noise_frequency;
	float intake_noise_filter_frequency;
	float intake_valve_shift;
	float exhaust_valve_shift;
	float crankshaft_fluctuation;
	float crankshaft_fluctuation_frequency;
	float crankshaft_fluctuation_filter_frequency;

	// Muffler params
	float straight_pipe_extractor_side_refl;
	float straight_pipe_muffler_side_refl;
	float straight_pipe_length;
	float output_side_refl;
	Array muffler_elements_output;

	// Cylinder params
	float cylinder_intake_opened_refl;
	float cylinder_intake_closed_refl;
	float cylinder_exhaust_opened_refl;
	float cylinder_exhaust_closed_refl;
	float cylinder_intake_open_end_refl;
	float cylinder_extractor_open_end_refl;
	Array cylinder_elements;

private:
	void on_muffler_changed() {
		engine_dirty = true;
		emit_changed();
	}

	void on_cylinder_changed() {
		engine_dirty = true;
		emit_changed();
	}

	void update_muffler_elements(Array new_elements);
	void update_cylinder_elements(Array new_elements);

	void build_engine();
	void gen(float &intake_channel, float &vibrations_channel, float &exhaust_channel, bool &channels_dampened);
public:
	static void _register_methods();

	//EngineMain *get_engine();

	bool is_engine_dirty() {return engine_dirty;}
	bool is_engine_valid() {
		if (engine_dirty) {
			build_engine();
		}
		return engine_valid;
	}

	void mark_dirty() {
		engine_dirty = true;
		emit_changed();
	}

	// Generation
	void clear_buffer();
	void fill_buffer(float *p_buffer, int p_num_frames, int p_num_channels);
	void fill_channel_buffers(float *p_intake_buffer, float *p_vibration_buffer, float *p_exhaust_buffer, int p_num_frames, int p_num_channels);
	void skip_frames(int p_num_frames);

	// Mixer
	void set_rpm(float p_rpm) {
		rpm = p_rpm;
		emit_changed();
	}
	float get_rpm() const {return rpm;}

	void set_volume(float p_volume) {
		volume = p_volume;
		emit_changed();
	}
	float get_volume() const {return volume;}

	void set_intake_volume(float p_volume) {
		intake_volume = p_volume;
		emit_changed();
	}
	float get_intake_volume() const {return intake_volume;}

	void set_exhaust_volume(float p_volume) {
		exhaust_volume = p_volume;
		emit_changed();
	}
	float get_exhaust_volume() const {return exhaust_volume;}

	void set_vibrations_volume(float p_volume) {
		vibrations_volume = p_volume;
		emit_changed();
	}
	float get_vibrations_volume() const {return vibrations_volume;}

	void set_dc_filter_frequency(float p_freq) {
		dc_filter_frequency = p_freq;
		emit_changed();
	}
	float get_dc_filter_frequency() const {return dc_filter_frequency;}

	bool get_waveguides_dampened() const {return waveguides_dampened;}

	void set_sample_rate(uint32_t p_rate) {
		sample_rate = p_rate;
		mark_dirty();
	}
	uint32_t get_sample_rate() const {return sample_rate;}

	// Engine params
	void set_vibrations_filter_frequency(float p_frequency) {
		vibrations_filter_frequency = p_frequency;
		mark_dirty();
	}
	float get_vibrations_filter_frequency() const {return vibrations_filter_frequency;}

	void set_intake_noise_factor(float p_factor) {
		intake_noise_factor = p_factor;
		mark_dirty();
	}
	float get_intake_noise_factor() const {return intake_noise_factor;}

	void set_intake_noise_frequency(float p_frequency) {
		intake_noise_frequency = p_frequency;
		mark_dirty();
	}
	float get_intake_noise_frequency() const {return intake_noise_frequency;}

	void set_intake_noise_filter_frequency(float p_frequency) {
		intake_noise_filter_frequency = p_frequency;
		mark_dirty();
	}
	float get_intake_noise_filter_frequency() const {return intake_noise_filter_frequency;}

	void set_intake_valve_shift(float p_shift) {
		intake_valve_shift = p_shift;
		mark_dirty();
	}
	float get_intake_valve_shift() const {return intake_valve_shift;}

	void set_exhaust_valve_shift(float p_shift) {
		exhaust_valve_shift = p_shift;
		mark_dirty();
	}
	float get_exhaust_valve_shift() const {return exhaust_valve_shift;}

	void set_crankshaft_fluctuation(float p_fluctuation) {
		crankshaft_fluctuation = p_fluctuation;
		mark_dirty();
	}
	float get_crankshaft_fluctuation() const {return crankshaft_fluctuation;}

	void set_crankshaft_fluctuation_frequency(float p_frequency) {
		crankshaft_fluctuation_frequency = p_frequency;
		mark_dirty();
	}
	float get_crankshaft_fluctuation_frequency() const {return crankshaft_fluctuation_frequency;}

	void set_crankshaft_fluctuation_filter_frequency(float p_frequency) {
		crankshaft_fluctuation_filter_frequency = p_frequency;
		mark_dirty();
	}
	float get_crankshaft_fluctuation_filter_frequency() const {return crankshaft_fluctuation_filter_frequency;}

	// Muffler params
	void set_straight_pipe_extractor_side_refl(float p_factor) {
		straight_pipe_extractor_side_refl = p_factor;
		mark_dirty();
	}
	float get_straight_pipe_extractor_side_refl() const {return straight_pipe_extractor_side_refl;}

	void set_straight_pipe_muffler_side_refl(float p_factor) {
		straight_pipe_muffler_side_refl = p_factor;
		mark_dirty();
	}
	float get_straight_pipe_muffler_side_refl() const {return straight_pipe_muffler_side_refl;}

	void set_straight_pipe_length(float p_factor) {
		straight_pipe_length = p_factor;
		mark_dirty();
	}
	float get_straight_pipe_length() const {return straight_pipe_length;}

	void set_output_side_refl(float p_factor) {
		output_side_refl = p_factor;
		mark_dirty();
	}
	float get_output_side_refl() const {return output_side_refl;}

	void set_muffler_elements_output(Array p_elements) {
		update_muffler_elements(p_elements);
	}
	Array get_muffler_elements_output() const {return muffler_elements_output;}

	// Cylinder params
	void set_cylinder_intake_opened_refl(float p_factor) {
		cylinder_intake_opened_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_intake_opened_refl() const {return cylinder_intake_opened_refl;}

	void set_cylinder_intake_closed_refl(float p_factor) {
		cylinder_intake_closed_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_intake_closed_refl() const {return cylinder_intake_closed_refl;}
	
	void set_cylinder_exhaust_opened_refl(float p_factor) {
		cylinder_exhaust_opened_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_exhaust_opened_refl() const {return cylinder_exhaust_opened_refl;}

	void set_cylinder_exhaust_closed_refl(float p_factor) {
		cylinder_exhaust_closed_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_exhaust_closed_refl() const {return cylinder_exhaust_closed_refl;}

	void set_cylinder_intake_open_end_refl(float p_factor) {
		cylinder_intake_open_end_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_intake_open_end_refl() const {return cylinder_intake_open_end_refl;}

	void set_cylinder_extractor_open_end_refl(float p_factor) {
		cylinder_extractor_open_end_refl = p_factor;
		mark_dirty();
	}
	float get_cylinder_extractor_open_end_refl() const {return cylinder_extractor_open_end_refl;}

	/*void set_cylinder_ignition_time(float p_factor) {
		cylinder_ignition_time = p_factor;
		mark_dirty();
	}
	float get_cylinder_ignition_time() const {return cylinder_ignition_time;}*/

	void set_cylinder_elements(Array p_elements) {
		update_cylinder_elements(p_elements);
	}
	Array get_cylinder_elements() const {return cylinder_elements;}

	void _init();

	EngineConfig();
	~EngineConfig();
};

class EngineCylinderConfig : public Resource {
	GODOT_CLASS(EngineCylinderConfig, Resource);
private:
	float piston_motion_factor;
	float ignition_factor;
	float ignition_time;
	float intake_pipe_length;
	float exhaust_pipe_length;
	float extractor_pipe_length;
	float crank_offset;
public:
	static void _register_methods();
	static EngineCylinderConfig *create(
		float piston_motion_factor, float ignition_factor, float ignition_time,
		float intake_pipe_length, float exhaust_pipe_length, float extractor_pipe_length,
		float crank_offset
	) {
		EngineCylinderConfig *cyl = EngineCylinderConfig::_new();

		cyl->piston_motion_factor = piston_motion_factor;
		cyl->ignition_factor = ignition_factor;
		cyl->ignition_time = ignition_time;
		cyl->intake_pipe_length = intake_pipe_length;
		cyl->exhaust_pipe_length = exhaust_pipe_length;
		cyl->extractor_pipe_length = extractor_pipe_length;
		cyl->crank_offset = crank_offset;

		return cyl;
	}

	void set_piston_motion_factor(float p_factor) {
		piston_motion_factor = p_factor;
		emit_changed();
	}
	float get_piston_motion_factor() const {return piston_motion_factor;}

	void set_ignition_factor(float p_factor) {
		ignition_factor = p_factor;
		emit_changed();
	}
	float get_ignition_factor() const {return ignition_factor;}

	void set_ignition_time(float p_time) {
		ignition_time = p_time;
		emit_changed();
	}
	float get_ignition_time() const {return ignition_time;}

	void set_intake_pipe_length(float p_length) {
		intake_pipe_length = p_length;
		emit_changed();
	}
	float get_intake_pipe_length() const {return intake_pipe_length;}

	void set_exhaust_pipe_length(float p_length) {
		exhaust_pipe_length = p_length;
		emit_changed();
	}
	float get_exhaust_pipe_length() const {return exhaust_pipe_length;}

	void set_extractor_pipe_length(float p_length) {
		extractor_pipe_length = p_length;
		emit_changed();
	}
	float get_extractor_pipe_length() const {return extractor_pipe_length;}

	void set_crank_offset(float p_off) {
		crank_offset = p_off;
		emit_changed();
	}
	float get_crank_offset() const {return crank_offset;}

	void _init();

	EngineCylinderConfig();
	~EngineCylinderConfig() {}
};

class EngineMufflerConfig : public Resource {
	GODOT_CLASS(EngineMufflerConfig, Resource);
private:
	float cavity_length;
public:
	static void _register_methods();
	static EngineMufflerConfig *create(float cavity_length) {
		EngineMufflerConfig *muf = EngineMufflerConfig::_new();
		muf->cavity_length = cavity_length;
		return muf;
	}

	void set_cavity_length(float p_length) {
		cavity_length = p_length;
		emit_changed();
	}
	float get_cavity_length() const {return cavity_length;}

	void _init();

	EngineMufflerConfig();
	~EngineMufflerConfig() {}
};

}


#endif // ENGINE_CONFIG_H