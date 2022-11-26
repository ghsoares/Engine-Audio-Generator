#include "engine_audio_recorder.h"
#include <GodotGlobal.hpp>
#include <PoolArrays.hpp>
#include <iostream>

using namespace godot;

template <typename T>
static inline T modulo(T a, T b) {
	return (a % b + b) % b;
}

static inline void pack_pcm16(uint16_t *pcm_data, float *frames, int num_frames, int num_channels) {
	for (int i = 0; i < num_frames; i++) {
		for (int j = 0; j < num_channels; j++) {
			int16_t v = (int)(frames[i * num_channels + j] * 32768);
			v = v < -32768 ? -32768 : (v > 32767 ? 32767 : v);

			pcm_data[i * num_channels + j] = 32768;
		}
	}
}

template <typename T>
static inline void v32_to_u16(T val, uint16_t &b0, uint16_t &b1) {
	uint16_t *val_ptr = (uint16_t *)(&val);

	b0 = val_ptr[0];
	b1 = val_ptr[1];
}

static inline void set_buffer_samples_crossfaded(
	float *p_audio_frames, uint16_t *p_buffer,
	int p_num_frames, int p_num_channels,
	int p_fade_frames
) {
	int fade_start = p_num_frames - p_fade_frames;
	for (int i = 0; i < p_num_frames - p_fade_frames; i++) {
		for (int j = 0; j < p_num_channels; j++) {
			float frame = p_audio_frames[i * p_num_channels + j];

			if (i < p_fade_frames) {
				float fade = i / (float)p_fade_frames;
				float fade_frame = p_audio_frames[(fade_start + i) * p_num_channels + j];
				frame = frame * fade + fade_frame * (1 - fade);
			}

			frame = Math::clamp(frame, -1.0f, 1.0f);
			int framei = (int)(frame * 32768);
			framei = framei < -32768 ? -32768 : (framei > 32767 ? 32767 : framei);

			p_buffer[i * p_num_channels + j] = (uint16_t)framei;
		}
	}
}

void EngineAudioRecorder::record() {
	ERR_FAIL_COND(!engine_config.is_valid());
	ERR_FAIL_COND(!engine_config->is_engine_valid());

	Ref<EngineConfig> config_duplicate = engine_config->duplicate();

	ERR_FAIL_COND(!config_duplicate.is_valid());
	ERR_FAIL_COND(!config_duplicate->is_engine_valid());

	const float min_secs = 1.0f / 120.0f;

	uint32_t sample_rate = config_duplicate->get_sample_rate();
	
	config_duplicate->clear_buffer();

	std::vector<float> crankshaft_frames;
	std::vector<float> ignition_frames;
	std::vector<float> exhaust_frames;
	int frame_off = 0;
	int total_frames = 0;
	int buffer_frames = 0;

	for (int i = 0; i < sample_count; i++) {
		float splf = i / (sample_count > 1 ? sample_count - 1.0f : 1.0f);
		float rpm = min_rpm + (top_rpm - min_rpm) * splf;
		float rps = rpm * min_secs;
		int cycles = (int)Math::max(duration_per_sample * rps, 1.0f);
		int frames = (int)Math::max((cycles / rps) * sample_rate, 1.0f);
		int preheat_cycles = (int)Math::max(preheat_time * rps, 1.0f);
		int preheat_frames = (int)Math::max((preheat_cycles / rps) * sample_rate, 1.0f);
		int fade_cycles = (int)(fade_time * rps);
		int fade_frames = (int)((fade_cycles / rps) * sample_rate);
		//int off = total_frames;
		total_frames += frames + fade_frames + padding_frames;
		buffer_frames += frames + padding_frames;

		crankshaft_frames.resize(total_frames * 2);
		ignition_frames.resize(total_frames * 2);
		exhaust_frames.resize(total_frames * 2);

		config_duplicate->set_rpm(rpm);
		if (i == 0) config_duplicate->skip_frames(preheat_frames);
		config_duplicate->fill_channel_buffers(
			&ignition_frames[frame_off * 2], &crankshaft_frames[frame_off * 2], &exhaust_frames[frame_off * 2],
			frames + fade_frames, 2
		);
		frame_off = total_frames;
	}
	/*
	Header:
		1 - Engine audio file identifier (single frame with values L: 0x5555 R: 0xAAAA)
		2 - Engine audio file version (single frame)
		v0:
			3 - Data size
			4 - Number of samples
			5 - Padding
			For each sample:
				1 - RPM
				2 - Start offset
				3 - End offset
			Rest - audio data
	*/

	// Data size
	int data_size = buffer_frames * 4;

	// Include header data size
	if (include_audio_header) {
		data_size += 5 * 4 + sample_count * 3 * 4 + padding_frames * 4;
	}

	PoolByteArray crankshaft_data;
	PoolByteArray ignition_data;
	PoolByteArray exhaust_data;
	crankshaft_data.resize(data_size);
	ignition_data.resize(data_size);
	exhaust_data.resize(data_size);
	PoolByteArray::Write crankshaft_data_write = crankshaft_data.write();
	PoolByteArray::Write ignition_data_write = ignition_data.write();
	PoolByteArray::Write exhaust_data_write = exhaust_data.write();
	uint16_t *crankshaft_buffer = (uint16_t *)crankshaft_data_write.ptr();
	uint16_t *ignition_buffer = (uint16_t *)ignition_data_write.ptr();
	uint16_t *exhaust_buffer = (uint16_t *)exhaust_data_write.ptr();

	int off = 0;

	// Include header data size
	if (include_audio_header) {
		#define SET_BUFFER(i, data) {		\
			crankshaft_buffer[i] = data; 	\
			ignition_buffer[i] = data; 		\
			exhaust_buffer[i] = data; 		\
		}

		#define SKIP_BUFFER(i) {			\
			crankshaft_buffer += i;			\
			ignition_buffer += i;			\
			exhaust_buffer += i;			\
		}

		// Engine audio file identifier
		SET_BUFFER(0, 0x5555);
		SET_BUFFER(1, 0xAAAA);

		// Engine audio version
		SET_BUFFER(2, 0x0000);
		SET_BUFFER(3, 0x0000);

		// Data size
		SET_BUFFER(4, (uint16_t)(buffer_frames * 4));
		SET_BUFFER(5, (uint16_t)((buffer_frames * 4) >> 16));

		// Number of samples
		SET_BUFFER(6, (uint16_t)(sample_count));
		SET_BUFFER(7, (uint16_t)(sample_count >> 16));

		// Padding
		SET_BUFFER(8, (uint16_t)(padding_frames));
		SET_BUFFER(9, (uint16_t)(padding_frames >> 19));

		SKIP_BUFFER(10);
		int sample_off = 0;
		int total_sample_frames = 0;

		// For each sample
		for (int i = 0; i < sample_count; i++) {
			float splf = i / (sample_count > 1 ? sample_count - 1.0f : 1.0f);
			float rpm = min_rpm + (top_rpm - min_rpm) * splf;
			float rps = rpm * min_secs;
			int cycles = (int)Math::max(duration_per_sample * rps, 1.0f);
			int frames = (int)Math::max((cycles / rps) * sample_rate, 1.0f);
			total_sample_frames += frames + padding_frames;

			// Reinterpret rpm float bits as integer
			int32_t rpmi;
			memcpy(&rpmi, &rpm, sizeof(float));

			// Add rpm
			SET_BUFFER(0, (uint16_t)(rpmi));
			SET_BUFFER(1, (uint16_t)(rpmi >> 16));

			// Add start offset
			SET_BUFFER(2, (uint16_t)(sample_off));
			SET_BUFFER(3, (uint16_t)(sample_off >> 16));

			// Add end offset
			SET_BUFFER(4, (uint16_t)((sample_off + frames)));
			SET_BUFFER(5, (uint16_t)((sample_off + frames) >> 16));

			SKIP_BUFFER(6);

			sample_off = total_sample_frames;
		}

		// Add padding
		for (int i = 0; i < padding_frames; i++) {
			SET_BUFFER(0, 0);
			SET_BUFFER(1, 0);
			SKIP_BUFFER(2);
		}

		#undef SKIP_BUFFER
		#undef SET_BUFFER
	}

	int buffer_off = 0;
	frame_off = 0;

	for (int i = 0; i < sample_count; i++) {
		float splf = i / (sample_count > 1 ? sample_count - 1.0f : 1.0f);
		float rpm = min_rpm + (top_rpm - min_rpm) * splf;
		float rps = rpm * min_secs;
		int cycles = (int)Math::max(duration_per_sample * rps, 1.0f);
		int frames = (int)Math::max((cycles / rps) * sample_rate, 1.0f);
		int fade_cycles = (int)(fade_time * rps);
		int fade_frames = (int)((fade_cycles / rps) * sample_rate);

		Godot::print("{0}", fade_frames);

		set_buffer_samples_crossfaded(
			&crankshaft_frames[frame_off * 2], &crankshaft_buffer[buffer_off * 2], 
			frames + fade_frames, 2, fade_frames
		);
		set_buffer_samples_crossfaded(
			&ignition_frames[frame_off * 2], &ignition_buffer[buffer_off * 2], 
			frames + fade_frames, 2, fade_frames
		);
		set_buffer_samples_crossfaded(
			&exhaust_frames[frame_off * 2], &exhaust_buffer[buffer_off * 2], 
			frames + fade_frames, 2, fade_frames
		);

		frame_off += frames + fade_frames + padding_frames;
		buffer_off += frames + padding_frames;
	}

	crankshaft_recording.instance();
	ignition_recording.instance();
	exhaust_recording.instance();

	crankshaft_recording->set_format(AudioStreamSample::FORMAT_16_BITS);
	ignition_recording->set_format(AudioStreamSample::FORMAT_16_BITS);
	exhaust_recording->set_format(AudioStreamSample::FORMAT_16_BITS);

	crankshaft_recording->set_mix_rate((int)sample_rate);
	ignition_recording->set_mix_rate((int)sample_rate);
	exhaust_recording->set_mix_rate((int)sample_rate);

	crankshaft_recording->set_stereo(true);
	ignition_recording->set_stereo(true);
	exhaust_recording->set_stereo(true);

	crankshaft_recording->set_data(crankshaft_data);
	ignition_recording->set_data(ignition_data);
	exhaust_recording->set_data(exhaust_data);
}

void EngineAudioRecorder::_init() {
	engine_config = Ref<EngineConfig>();
	crankshaft_recording = Ref<AudioStreamSample>();
	ignition_recording = Ref<AudioStreamSample>();
	exhaust_recording = Ref<AudioStreamSample>();
	min_rpm = 1000;
	top_rpm = 4000;
	duration_per_sample = 1.0f;
	preheat_time = 1.0f;
	fade_time = 0.1f;
	sample_count = 32;
	padding_frames = 8;
	include_audio_header = true;
}

void EngineAudioRecorder::_register_methods() {
	register_property<EngineAudioRecorder, Ref<EngineConfig>>(
		"engine_configuration", 
		&EngineAudioRecorder::set_engine_configuration,
		&EngineAudioRecorder::get_engine_configuration,
		Ref<EngineConfig>()
	);
	register_property<EngineAudioRecorder, float>(
		"min_rpm", 
		&EngineAudioRecorder::set_min_rpm,
		&EngineAudioRecorder::get_min_rpm,
		60.0f
	);
	register_property<EngineAudioRecorder, float>(
		"top_rpm", 
		&EngineAudioRecorder::set_top_rpm,
		&EngineAudioRecorder::get_top_rpm,
		60.0f
	);
	register_property<EngineAudioRecorder, float>(
		"duration_per_sample", 
		&EngineAudioRecorder::set_duration_per_sample,
		&EngineAudioRecorder::get_duration_per_sample,
		1.0f
	);
	register_property<EngineAudioRecorder, float>(
		"preheat_time", 
		&EngineAudioRecorder::set_preheat_time,
		&EngineAudioRecorder::get_preheat_time,
		1.0f
	);
	register_property<EngineAudioRecorder, float>(
		"fade_time", 
		&EngineAudioRecorder::set_fade_time,
		&EngineAudioRecorder::get_fade_time,
		0.1f
	);
	register_property<EngineAudioRecorder, int>(
		"sample_count", 
		&EngineAudioRecorder::set_sample_count,
		&EngineAudioRecorder::get_sample_count,
		32
	);
	register_property<EngineAudioRecorder, int>(
		"padding_frames", 
		&EngineAudioRecorder::set_padding_frames,
		&EngineAudioRecorder::get_padding_frames,
		8
	);
	register_property<EngineAudioRecorder, bool>(
		"include_audio_header", 
		&EngineAudioRecorder::set_include_audio_header,
		&EngineAudioRecorder::get_include_audio_header,
		true
	);
	
	register_method("record", &EngineAudioRecorder::record);
	register_method("get_crankshaft_recording", &EngineAudioRecorder::get_crankshaft_recording);
	register_method("get_ignition_recording", &EngineAudioRecorder::get_ignition_recording);
	register_method("get_exhaust_recording", &EngineAudioRecorder::get_exhaust_recording);
}

EngineAudioRecorder::EngineAudioRecorder() {
	
}

EngineAudioRecorder::~EngineAudioRecorder() {
	
}

