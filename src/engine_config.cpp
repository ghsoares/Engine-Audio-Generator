#include "engine_config.h"
#include "engine_utils.h"

using namespace godot;

void EngineConfig::update_muffler_elements(Array new_elements) {
	uint32_t count = muffler_elements_output.size();
	for (uint32_t i = 0; i < count; i++) {
		EngineMufflerConfig *muf_res = Object::cast_to<EngineMufflerConfig>(muffler_elements_output[i]);
		ERR_FAIL_COND(!muf_res);

		if (muf_res->is_connected("changed", this, "on_muffler_changed")) {
			muf_res->disconnect("changed", this, "on_muffler_changed");
		}
	}
	muffler_elements_output = new_elements;
	count = muffler_elements_output.size();
	for (uint32_t i = 0; i < count; i++) {
		EngineMufflerConfig *muf_res = Object::cast_to<EngineMufflerConfig>(muffler_elements_output[i]);
		ERR_FAIL_COND(!muf_res);

		muf_res->connect("changed", this, "on_muffler_changed");
	}
}

void EngineConfig::update_cylinder_elements(Array new_elements) {
	uint32_t count = cylinder_elements.size();
	for (uint32_t i = 0; i < count; i++) {
		EngineCylinderConfig *cyl_res = Object::cast_to<EngineCylinderConfig>(cylinder_elements[i]);
		ERR_FAIL_COND(!cyl_res);

		if (cyl_res->is_connected("changed", this, "on_cylinder_changed")) {
			cyl_res->disconnect("changed", this, "on_cylinder_changed");
		}
	}
	cylinder_elements = new_elements;
	count = cylinder_elements.size();
	for (uint32_t i = 0; i < count; i++) {
		EngineCylinderConfig *cyl_res = Object::cast_to<EngineCylinderConfig>(cylinder_elements[i]);
		ERR_FAIL_COND(!cyl_res);

		cyl_res->connect("changed", this, "on_cylinder_changed");
	}
}

void EngineConfig::build_engine() {
	engine_valid = false;
	engine_dirty = true;

	if (!engine) {
		engine = new EngineMain();
	}

	if (!dc_filter) {
		dc_filter = new LowPassFilter(
			dc_filter_frequency, sample_rate
		);
	} else {
		dc_filter->modify(
			dc_filter_frequency, sample_rate
		);
	}
	
	engine->intake_noise_factor = intake_noise_factor;
	engine->intake_valve_shift = intake_valve_shift;
	engine->exhaust_valve_shift = exhaust_valve_shift;
	engine->crankshaft_fluctuation = crankshaft_fluctuation;

	if (!engine->intake_noise) {
		engine->intake_noise = new Noise();
		// engine->intake_noise->set_octaves(3);
	}
	if (!engine->crankshaft_noise) {
		engine->crankshaft_noise = new Noise();
		// engine->crankshaft_noise->set_octaves(3);
	}
	
	if (!engine->intake_noise_lp) {
		engine->intake_noise_lp = new LowPassFilter(intake_noise_filter_frequency, sample_rate);
	} else {
		engine->intake_noise_lp->modify(intake_noise_filter_frequency, sample_rate);
	}

	if (!engine->vibration_filter) {
		engine->vibration_filter = new LowPassFilter(vibrations_filter_frequency, sample_rate);
	} else {
		engine->vibration_filter->modify(vibrations_filter_frequency, sample_rate);
	}

	if (!engine->crankshaft_fluctuation_lp) {
		engine->crankshaft_fluctuation_lp = new LowPassFilter(crankshaft_fluctuation_filter_frequency, sample_rate);
	} else {
		engine->crankshaft_fluctuation_lp->modify(crankshaft_fluctuation_filter_frequency, sample_rate);
	}

	// Build cylinders
	{
		uint32_t cylinder_count = cylinder_elements.size();
		size_t prev_cylinder_count = engine->cylinders.size();

		if (cylinder_count < prev_cylinder_count) {
			for (uint32_t i = cylinder_count; i < prev_cylinder_count; i++) {
				delete engine->cylinders[i];
			}
		}

		engine->cylinders.resize(cylinder_count, nullptr);

		for (uint32_t i = 0; i < cylinder_count; i++) {
			EngineCylinderConfig *cyl_res = Object::cast_to<EngineCylinderConfig>(cylinder_elements[i]);
			ERR_FAIL_COND(!cyl_res);

			EngineCylinder *cyl = nullptr;

			if (i < prev_cylinder_count) {
				cyl = engine->cylinders[i];
			}

			if (!cyl) {
				cyl = new EngineCylinder();
			}

			cyl->piston_motion_factor = cyl_res->get_piston_motion_factor();
			cyl->ignition_factor = cyl_res->get_ignition_factor();
			cyl->crank_offset = cyl_res->get_crank_offset();
			cyl->ignition_time = cyl_res->get_ignition_time();
			cyl->intake_open_refl = cylinder_intake_opened_refl;
			cyl->intake_closed_refl = cylinder_intake_closed_refl;
			cyl->exhaust_open_refl = cylinder_exhaust_opened_refl;
			cyl->exhaust_closed_refl = cylinder_exhaust_closed_refl;

			if (!cyl->intake_waveguide) {
				cyl->intake_waveguide = new WaveGuide(
					distance_to_samples(cyl_res->get_intake_pipe_length(), sample_rate),
					1.0f,
					cylinder_intake_open_end_refl,
					sample_rate
				);
			} else {
				cyl->intake_waveguide->modify(
					distance_to_samples(cyl_res->get_intake_pipe_length(), sample_rate),
					1.0f,
					cylinder_intake_open_end_refl,
					sample_rate
				);
			}

			if (!cyl->exhaust_waveguide) {
				cyl->exhaust_waveguide = new WaveGuide(
					distance_to_samples(cyl_res->get_exhaust_pipe_length(), sample_rate),
					0.71f,
					0.06f,
					sample_rate
				);
			} else {
				cyl->exhaust_waveguide->modify(
					distance_to_samples(cyl_res->get_exhaust_pipe_length(), sample_rate),
					0.71f,
					0.06f,
					sample_rate
				);
			}

			if (!cyl->extractor_waveguide) {
				cyl->extractor_waveguide = new WaveGuide(
					distance_to_samples(cyl_res->get_extractor_pipe_length(), sample_rate),
					0.0,
					cylinder_extractor_open_end_refl,
					sample_rate
				);
			} else {
				cyl->extractor_waveguide->modify(
					distance_to_samples(cyl_res->get_extractor_pipe_length(), sample_rate),
					0.0,
					cylinder_extractor_open_end_refl,
					sample_rate
				);
			}

			engine->cylinders[i] = cyl;
		}
	}

	// Build mufflers
	{
		if (!engine->muffler) {
			engine->muffler = new EngineMuffler();
		}
		EngineMuffler *muffler = engine->muffler;

		uint32_t muffler_count = muffler_elements_output.size();
		size_t prev_muffler_count = muffler->muffler_elements.size();

		if (muffler_count < prev_muffler_count) {
			for (uint32_t i = muffler_count; i < prev_muffler_count; i++) {
				delete muffler->muffler_elements[i];
			}
		}

		if (!muffler->straight_pipe) {
			muffler->straight_pipe = new WaveGuide(
				distance_to_samples(straight_pipe_length, sample_rate),
				straight_pipe_extractor_side_refl,
				straight_pipe_muffler_side_refl,
				sample_rate
			);
		} else {
			muffler->straight_pipe->modify(
				distance_to_samples(straight_pipe_length, sample_rate),
				straight_pipe_extractor_side_refl,
				straight_pipe_muffler_side_refl,
				sample_rate
			);
		}

		muffler->muffler_elements.resize(muffler_count, nullptr);

		for (uint32_t i = 0; i < muffler_count; i++) {
			EngineMufflerConfig *muf_res = Object::cast_to<EngineMufflerConfig>(muffler_elements_output[i]);
			ERR_FAIL_COND(!muf_res);

			WaveGuide *muf = nullptr;

			if (i < prev_muffler_count) {
				muf = muffler->muffler_elements[i];
				if (muf) {
					muf->modify(
						distance_to_samples(muf_res->get_cavity_length(), sample_rate),
						0.0,
						output_side_refl,
						sample_rate
					);
				}
			}

			if (!muf) {
				muf = new WaveGuide(
					distance_to_samples(muf_res->get_cavity_length(), sample_rate),
					0.0,
					output_side_refl,
					sample_rate
				);
			}

			muffler->muffler_elements[i] = muf;
		}

		engine->muffler = muffler;
	}

	engine_dirty = false;
	engine_valid = true;
}

void EngineConfig::gen(float &intake_channel, float &vibrations_channel, float &exhaust_channel, bool &channels_dampened) {
	ERR_FAIL_COND(!engine_valid);

	// float noise_cos = Math::cos(engine->noise_pos * (float)Math_TAU) * 500.f;
	// float noise_sin = Math::cos(engine->noise_pos * (float)Math_TAU) * 500.f;
	// engine->intake_noise->set_repeat((int32_t)(500.f * intake_noise_frequency));
	// engine->crankshaft_noise->set_repeat((int32_t)(500.f * crankshaft_fluctuation_frequency));
	float intake_noise = engine->intake_noise_lp->filter(
		engine->intake_noise->next_f32()
		// engine->intake_noise->get(
		// 	engine->noise_pos * 500.f * intake_noise_frequency,
		// 	0.f
		// )
	) * engine->intake_noise_factor;

	float vibrations = 0.0;

	size_t cylinder_count = engine->cylinders.size();
	float num_cyl = (float)cylinder_count;

	float last_exhaust_collector = engine->exhaust_collector / num_cyl;
	engine->exhaust_collector = 0.0;
	engine->intake_collector = 0.0;

	float crankshaft_fluctuation_off = engine->crankshaft_fluctuation_lp->filter(
		engine->crankshaft_noise->next_f32()
		// engine->crankshaft_noise->get(
		// 	engine->noise_pos * 500.f * crankshaft_fluctuation_frequency,
		// 	0.f
		// )
	);

	bool cylinder_dampened = false;

	for (size_t i = 0; i < cylinder_count; i++) {
		EngineCylinder *cylinder = engine->cylinders[i];

		float cyl_intake;
		float cyl_exhaust;
		float cyl_vib;
		bool cyl_dampened;
		cylinder->pop(
			engine->crankshaft_pos +
				engine->crankshaft_fluctuation * crankshaft_fluctuation_off,
			last_exhaust_collector,
			engine->intake_valve_shift,
			engine->exhaust_valve_shift,
			cyl_intake, cyl_exhaust, cyl_vib, cyl_dampened
		);

		engine->intake_collector += cyl_intake;
		engine->exhaust_collector += cyl_exhaust;

		vibrations += cyl_vib;
		cylinder_dampened = cylinder_dampened || cyl_dampened;
	}

	float straight_pipe_c1, straight_pipe_c0;
	bool straight_pipe_dampened;
	engine->muffler->straight_pipe->pop(straight_pipe_c1, straight_pipe_c0, straight_pipe_dampened);

	float muffler_c1 = 0.0, muffler_c0 = 0.0;
	bool muffler_dampened = false;

	size_t muffler_count = engine->muffler->muffler_elements.size();

	for (size_t i = 0; i < muffler_count; i++) {
		WaveGuide *muffler_line = engine->muffler->muffler_elements[i];
		float muffler_line_c1, muffler_line_c0;
		bool muffler_line_dampened;
		muffler_line->pop(muffler_line_c1, muffler_line_c0, muffler_line_dampened);
		muffler_c1 += muffler_line_c1;
		muffler_c0 += muffler_line_c0;
		muffler_dampened = muffler_dampened || muffler_line_dampened;
	}

	for (size_t i = 0; i < cylinder_count; i++) {
		EngineCylinder *cylinder = engine->cylinders[i];

		cylinder->push(
			engine->intake_collector / num_cyl +
				intake_noise * intake_valve(
					Math::fmod(engine->crankshaft_pos + cylinder->crank_offset, 1.0f)
				)
		);
	}

	engine->muffler->straight_pipe->push(
		engine->exhaust_collector, muffler_c1
	);
	engine->exhaust_collector += straight_pipe_c1;

	float num_muffler = (float)muffler_count;

	for (size_t i = 0; i < muffler_count; i++) {
		WaveGuide *muffler_delay_line = engine->muffler->muffler_elements[i];
		muffler_delay_line->push(straight_pipe_c0 / num_muffler, 0.0);
	}

	vibrations = engine->vibration_filter->filter(vibrations);

	intake_channel = engine->intake_collector;
	vibrations_channel = vibrations;
	exhaust_channel = muffler_c0;
	channels_dampened = straight_pipe_dampened || cylinder_dampened;
}

void EngineConfig::clear_buffer() {
	if (engine_dirty) {
		build_engine();
	}
	ERR_FAIL_COND(!engine_valid);

	engine->clear();
	dc_filter->clear();
}

void EngineConfig::fill_buffer(float *p_buffer, int p_num_frames, int p_num_channels) {
	if (engine_dirty) {
		build_engine();
	}

	ERR_FAIL_COND(!engine_valid);

	dc_filter->modify(
		dc_filter_frequency, sample_rate
	);

	waveguides_dampened = false;

	float inc = rpm / (sample_rate * 120.f);
	float noise_inc = (rpm / (sample_rate * 120.f)) / 500.f;

	for (int frame = 0; frame < p_num_frames; frame++) {
		engine->crankshaft_pos = Math::fmod(engine->crankshaft_pos + inc, 1.f);
		engine->noise_pos = Math::fmod(engine->noise_pos + noise_inc, 1.f);

		float intake_channel;
		float vibrations_channel;
		float exhaust_channel;
		bool channels_dampened;

		gen(intake_channel, vibrations_channel, exhaust_channel, channels_dampened);
	
		intake_channel *= intake_volume;
		vibrations_channel *= vibrations_volume;
		exhaust_channel *= exhaust_volume;

		float mixed = (intake_channel + vibrations_channel + exhaust_channel) * volume;

		waveguides_dampened = waveguides_dampened || channels_dampened;

		mixed -= dc_filter->filter(mixed);

		for (int c = 0; c < p_num_channels; c++) {
			p_buffer[frame * p_num_channels + c] = mixed;
		}
	}
}

void EngineConfig::fill_channel_buffers(float *p_intake_buffer, float *p_vibration_buffer, float *p_exhaust_buffer, int p_num_frames, int p_num_channels) {
	if (engine_dirty) {
		build_engine();
	}

	ERR_FAIL_COND(!engine_valid);

	waveguides_dampened = false;

	float inc = rpm / (sample_rate * 120.f);
	float noise_inc = (rpm / (sample_rate * 120.f)) / 500.f;

	for (int frame = 0; frame < p_num_frames; frame++) {
		engine->crankshaft_pos = Math::fmod(engine->crankshaft_pos + inc, 1.f);
		engine->noise_pos = Math::fmod(engine->noise_pos + noise_inc, 1.f);

		float intake_channel;
		float vibrations_channel;
		float exhaust_channel;
		bool channels_dampened;

		gen(intake_channel, vibrations_channel, exhaust_channel, channels_dampened);
	
		intake_channel *= intake_volume * volume;
		vibrations_channel *= vibrations_volume * volume;
		exhaust_channel *= exhaust_volume * volume;

		waveguides_dampened = waveguides_dampened || channels_dampened;

		for (int c = 0; c < p_num_channels; c++) {
			//p_buffer[frame * p_num_channels + c] = mixed;
			p_intake_buffer[frame * p_num_channels + c] = intake_channel;
			p_vibration_buffer[frame * p_num_channels + c] = vibrations_channel;
			p_exhaust_buffer[frame * p_num_channels + c] = exhaust_channel;
		}
	}
}

void EngineConfig::skip_frames(int p_num_frames) {
	if (engine_dirty) {
		build_engine();
	}

	ERR_FAIL_COND(!engine_valid);
	
	dc_filter->modify(
		dc_filter_frequency, sample_rate
	);

	waveguides_dampened = false;

	float inc = rpm / (sample_rate * 120.f);

	for (int frame = 0; frame < p_num_frames; frame++) {
		engine->crankshaft_pos = Math::fmod(engine->crankshaft_pos + inc, 1.f);

		float intake_channel;
		float vibrations_channel;
		float exhaust_channel;
		bool channels_dampened;

		gen(intake_channel, vibrations_channel, exhaust_channel, channels_dampened);
	
		intake_channel *= intake_volume;
		vibrations_channel *= vibrations_volume;
		exhaust_channel *= exhaust_volume;

		float mixed = (intake_channel + vibrations_channel + exhaust_channel) * volume;

		waveguides_dampened = waveguides_dampened || channels_dampened;

		dc_filter->filter(mixed);
	}
}

void EngineConfig::_init() {
	
}

void EngineCylinderConfig::_init() {
	
}

void EngineMufflerConfig::_init() {
	
}

void EngineConfig::_register_methods() {
	register_property<EngineConfig, float>(
		"rpm", 
		&EngineConfig::set_rpm,
		&EngineConfig::get_rpm,
		1000.0f
	);
	register_property<EngineConfig, float>(
		"volume", 
		&EngineConfig::set_volume,
		&EngineConfig::get_volume,
		0.5f
	);
	register_property<EngineConfig, float>(
		"intake_volume", 
		&EngineConfig::set_intake_volume,
		&EngineConfig::get_intake_volume,
		0.5f
	);
	register_property<EngineConfig, float>(
		"exhaust_volume", 
		&EngineConfig::set_exhaust_volume,
		&EngineConfig::get_exhaust_volume,
		0.25f
	);
	register_property<EngineConfig, float>(
		"vibrations_volume", 
		&EngineConfig::set_vibrations_volume,
		&EngineConfig::get_vibrations_volume,
		0.1f
	);
	register_property<EngineConfig, float>(
		"dc_filter_frequency", 
		&EngineConfig::set_dc_filter_frequency,
		&EngineConfig::get_dc_filter_frequency,
		0.5f
	);
	register_property<EngineConfig, uint32_t>(
		"sample_rate", 
		&EngineConfig::set_sample_rate,
		&EngineConfig::get_sample_rate,
		20050
	);

	register_property<EngineConfig, float>(
		"vibrations_filter_frequency", 
		&EngineConfig::set_vibrations_filter_frequency,
		&EngineConfig::get_vibrations_filter_frequency,
		92.0f
	);
	register_property<EngineConfig, float>(
		"intake_noise_factor", 
		&EngineConfig::set_intake_noise_factor,
		&EngineConfig::get_intake_noise_factor,
		0.2f
	);
	register_property<EngineConfig, float>(
		"intake_noise_frequency", 
		&EngineConfig::set_intake_noise_frequency,
		&EngineConfig::get_intake_noise_frequency,
		100.0f
	);
	register_property<EngineConfig, float>(
		"intake_noise_filter_frequency", 
		&EngineConfig::set_intake_noise_filter_frequency,
		&EngineConfig::get_intake_noise_filter_frequency,
		10900.0f
	);
	register_property<EngineConfig, float>(
		"intake_valve_shift", 
		&EngineConfig::set_intake_valve_shift,
		&EngineConfig::get_intake_valve_shift,
		0.04f
	);
	register_property<EngineConfig, float>(
		"exhaust_valve_shift", 
		&EngineConfig::set_exhaust_valve_shift,
		&EngineConfig::get_exhaust_valve_shift,
		0.0f
	);
	register_property<EngineConfig, float>(
		"crankshaft_fluctuation", 
		&EngineConfig::set_crankshaft_fluctuation,
		&EngineConfig::get_crankshaft_fluctuation,
		0.3f
	);
	register_property<EngineConfig, float>(
		"crankshaft_fluctuation_frequency", 
		&EngineConfig::set_crankshaft_fluctuation_frequency,
		&EngineConfig::get_crankshaft_fluctuation_frequency,
		100.0f
	);
	register_property<EngineConfig, float>(
		"crankshaft_fluctuation_filter_frequency", 
		&EngineConfig::set_crankshaft_fluctuation_filter_frequency,
		&EngineConfig::get_crankshaft_fluctuation_filter_frequency,
		57.0f
	);

	register_property<EngineConfig, float>(
		"straight_pipe_extractor_side_refl", 
		&EngineConfig::set_straight_pipe_extractor_side_refl,
		&EngineConfig::get_straight_pipe_extractor_side_refl,
		0.06f
	);
	register_property<EngineConfig, float>(
		"straight_pipe_muffler_side_refl", 
		&EngineConfig::set_straight_pipe_muffler_side_refl,
		&EngineConfig::get_straight_pipe_muffler_side_refl,
		0.0f
	);
	register_property<EngineConfig, float>(
		"straight_pipe_length", 
		&EngineConfig::set_straight_pipe_length,
		&EngineConfig::get_straight_pipe_length,
		2.0f
	);
	register_property<EngineConfig, float>(
		"output_side_refl", 
		&EngineConfig::set_output_side_refl,
		&EngineConfig::get_output_side_refl,
		-0.14f
	);
	register_property<EngineConfig, Array>(
		"muffler_elements_output", 
		&EngineConfig::set_muffler_elements_output,
		&EngineConfig::get_muffler_elements_output,
		Array(), 
		GODOT_METHOD_RPC_MODE_DISABLED,
		GODOT_PROPERTY_USAGE_DEFAULT,
		GODOT_PROPERTY_HINT_TYPE_STRING,
		"17/19:EngineMufflerConfig"
	);

	register_property<EngineConfig, float>(
		"cylinder_intake_opened_refl", 
		&EngineConfig::set_cylinder_intake_opened_refl,
		&EngineConfig::get_cylinder_intake_opened_refl,
		0.0f
	);
	register_property<EngineConfig, float>(
		"cylinder_intake_closed_refl", 
		&EngineConfig::set_cylinder_intake_closed_refl,
		&EngineConfig::get_cylinder_intake_closed_refl,
		1.0f
	);
	register_property<EngineConfig, float>(
		"cylinder_exhaust_opened_refl", 
		&EngineConfig::set_cylinder_exhaust_opened_refl,
		&EngineConfig::get_cylinder_exhaust_opened_refl,
		0.0f
	);
	register_property<EngineConfig, float>(
		"cylinder_exhaust_closed_refl", 
		&EngineConfig::set_cylinder_exhaust_closed_refl,
		&EngineConfig::get_cylinder_exhaust_closed_refl,
		0.7f
	);
	register_property<EngineConfig, float>(
		"cylinder_intake_open_end_refl", 
		&EngineConfig::set_cylinder_intake_open_end_refl,
		&EngineConfig::get_cylinder_intake_open_end_refl,
		-0.75f
	);
	register_property<EngineConfig, float>(
		"cylinder_extractor_open_end_refl", 
		&EngineConfig::set_cylinder_extractor_open_end_refl,
		&EngineConfig::get_cylinder_extractor_open_end_refl,
		0.0f
	);
	register_property<EngineConfig, Array>(
		"cylinder_elements", 
		&EngineConfig::set_cylinder_elements,
		&EngineConfig::get_cylinder_elements,
		Array(), 
		GODOT_METHOD_RPC_MODE_DISABLED,
		GODOT_PROPERTY_USAGE_DEFAULT,
		GODOT_PROPERTY_HINT_TYPE_STRING,
		"17/19:EngineCylinderConfig"
	);

	register_method("clear_buffer", &EngineConfig::clear_buffer);
	register_method("skip_frames", &EngineConfig::skip_frames);

	register_method("on_cylinder_changed", &EngineConfig::on_cylinder_changed);
	register_method("on_muffler_changed", &EngineConfig::on_muffler_changed);

	/*
	"17/19:EngineCylinderConfig"*/
}

void EngineCylinderConfig::_register_methods() {
	register_property<EngineCylinderConfig, float>(
		"piston_motion_factor", 
		&EngineCylinderConfig::set_piston_motion_factor,
		&EngineCylinderConfig::get_piston_motion_factor,
		1.0f
	);
	register_property<EngineCylinderConfig, float>(
		"ignition_factor", 
		&EngineCylinderConfig::set_ignition_factor,
		&EngineCylinderConfig::get_ignition_factor,
		1.0f
	);
	register_property<EngineCylinderConfig, float>(
		"ignition_time", 
		&EngineCylinderConfig::set_ignition_time,
		&EngineCylinderConfig::get_ignition_time,
		0.3f
	);
	register_property<EngineCylinderConfig, float>(
		"intake_pipe_length", 
		&EngineCylinderConfig::set_intake_pipe_length,
		&EngineCylinderConfig::get_intake_pipe_length,
		0.08f
	);
	register_property<EngineCylinderConfig, float>(
		"exhaust_pipe_length", 
		&EngineCylinderConfig::set_exhaust_pipe_length,
		&EngineCylinderConfig::get_exhaust_pipe_length,
		0.1f
	);
	register_property<EngineCylinderConfig, float>(
		"extractor_pipe_length", 
		&EngineCylinderConfig::set_extractor_pipe_length,
		&EngineCylinderConfig::get_extractor_pipe_length,
		0.1f
	);
	register_property<EngineCylinderConfig, float>(
		"crank_offset", 
		&EngineCylinderConfig::set_crank_offset,
		&EngineCylinderConfig::get_crank_offset,
		0.0f
	);
}

void EngineMufflerConfig::_register_methods() {
	register_property<EngineMufflerConfig, float>(
		"cavity_length", 
		&EngineMufflerConfig::set_cavity_length,
		&EngineMufflerConfig::get_cavity_length,
		0.04f
	);
}

EngineConfig::EngineConfig() {
	rpm = 1000.f;
	volume = 0.5f;
	intake_volume = 0.5f;
	exhaust_volume = 0.25f;
	vibrations_volume = 0.1f;
	dc_filter_frequency = 0.5f;
	sample_rate = 20050;

	vibrations_filter_frequency = 92.0f;
	intake_noise_factor = 0.2f;
	intake_noise_frequency = 100.0f;
	intake_noise_filter_frequency = 10900.0f;
	intake_valve_shift = 0.04f;
	exhaust_valve_shift = 0.0f;
	crankshaft_fluctuation = 0.3f;
	crankshaft_fluctuation_frequency = 100.0f;
	crankshaft_fluctuation_filter_frequency = 57.0f;

	straight_pipe_extractor_side_refl = 0.06f;
	straight_pipe_muffler_side_refl = 0.0f;
	straight_pipe_length = 2.0f;
	output_side_refl = -0.14f;
	muffler_elements_output = Array();

	muffler_elements_output.append(EngineMufflerConfig::create(0.04f));
	muffler_elements_output.append(EngineMufflerConfig::create(0.06f));
	muffler_elements_output.append(EngineMufflerConfig::create(0.07f));
	muffler_elements_output.append(EngineMufflerConfig::create(0.09f));

	cylinder_intake_opened_refl = 0.04f;
	cylinder_intake_closed_refl = 1.0f;
	cylinder_exhaust_opened_refl = 0.0f;
	cylinder_exhaust_closed_refl = 0.7f;
	cylinder_intake_open_end_refl = -0.75f;
	cylinder_extractor_open_end_refl = 0.0f;
	cylinder_elements = Array();

	cylinder_elements.append(EngineCylinderConfig::create(
		2.43f, 5.f, 0.1f,
		0.08f, 0.1f, 0.1f,
		0.f / 8.f
	));
	cylinder_elements.append(EngineCylinderConfig::create(
		2.43f, 5.f, 0.1f,
		0.08f, 0.1f, 0.1f,
		1.5f / 8.f
	));
	cylinder_elements.append(EngineCylinderConfig::create(
		2.43f, 5.f, 0.1f,
		0.08f, 0.1f, 0.1f,
		2.5f / 8.f
	));
	cylinder_elements.append(EngineCylinderConfig::create(
		2.43f, 5.f, 0.1f,
		0.08f, 0.1f, 0.1f,
		4.f / 8.f
	));

	engine = nullptr;
	engine_valid = false;
	dc_filter = nullptr;
	engine_dirty = true;
}

EngineConfig::~EngineConfig() {
	if (engine) {
		delete engine;
	}

	if (dc_filter) {
		delete dc_filter;
	}
}

EngineCylinderConfig::EngineCylinderConfig() {
	piston_motion_factor = 2.43f;
	ignition_factor = 5.f;
	ignition_time = 0.1f;
	intake_pipe_length = 0.08f;
	exhaust_pipe_length = 0.1f;
	extractor_pipe_length = 0.1f;
	crank_offset = 0.0f;
}

EngineMufflerConfig::EngineMufflerConfig() {
	cavity_length = 0.04f;
}