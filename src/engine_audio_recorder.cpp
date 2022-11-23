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

void EngineAudioRecorder::record() {
	ERR_FAIL_COND(!engine_config.is_valid());
	ERR_FAIL_COND(!engine_config->is_engine_valid());

	Ref<EngineConfig> config_duplicate = engine_config->duplicate();

	ERR_FAIL_COND(!config_duplicate.is_valid());
	ERR_FAIL_COND(!config_duplicate->is_engine_valid());

	const float min_secs = 1.0f / 60.0f;

	uint32_t sample_rate = config_duplicate->get_sample_rate();
	int preheat_frames = (int)((preheat_cycles / (min_rpm * min_secs)) * sample_rate);

	config_duplicate->set_rpm(min_rpm);

	config_duplicate->clear_buffer();
	config_duplicate->skip_frames(preheat_frames);

	std::vector<float> audio_frames;
	int total_frames = 0;

	for (int i = 0; i < sample_count; i++) {
		float splf = i / (sample_count > 1 ? sample_count - 1.0f : 1.0f);
		float rpm = min_rpm + (top_rpm - min_rpm) * splf;
		float rps = rpm * min_secs;
		int cycles = (int)(duration_per_sample * rps);
		int frames = (int)((cycles / rps) * sample_rate);
		int off = total_frames;
		total_frames += frames;

		audio_frames.resize(total_frames * 2);

		config_duplicate->set_rpm(rpm);
		config_duplicate->fill_buffer(&audio_frames[off * 2], frames, 2);
	}
	/*
	Header:
		1 - Engine audio file identifier (single frame with values L: 0x5555 R: 0xAAAA)
		2 - Engine audio file version (single frame)
		v0:
			3 - Number of samples
			For each sample:
				1 - RPM
				2 - Start offset
				3 - End offset
	*/

	// Data size
	int data_size = total_frames * 4;

	// Include header data size
	if (include_audio_header) {
		data_size += 3 * 4 + sample_count * 3 * 4;
	}

	PoolByteArray data;
	data.resize(data_size);
	PoolByteArray::Write data_write = data.write();
	uint16_t *buffer = (uint16_t *)data_write.ptr();

	int off = 0;

	// Include header data size
	if (include_audio_header) {
		// Engine audio file identifier
		buffer[0] = 0x5555;
		buffer[1] = 0xAAAA;

		// Engine audio version
		buffer[2] = 0x0000;
		buffer[3] = 0x0000;

		// Number of samples
		buffer[4] = (uint16_t)(sample_count);
		buffer[5] = (uint16_t)(sample_count >> 16);

		off = 6;
		int total_sample_frames = 0;

		// For each sample
		for (int i = 0; i < sample_count; i++) {
			float splf = i / (sample_count > 1 ? sample_count - 1.0f : 1.0f);
			float rpm = min_rpm + (top_rpm - min_rpm) * splf;
			float rps = rpm * min_secs;
			int cycles = (int)(duration_per_sample * rps);
			int frames = (int)((cycles / rps) * sample_rate);
			int sample_off = total_sample_frames;
			total_sample_frames += frames;

			// Reinterpret rpm float bits as integer
			int32_t rpmi;
			memcpy(&rpmi, &rpm, sizeof(float));

			// Add rpm
			buffer[off + 0] = (uint16_t)(rpmi);
			buffer[off + 1] = (uint16_t)(rpmi >> 16);

			// Add start offset
			buffer[off + 2] = (uint16_t)(sample_off);
			buffer[off + 3] = (uint16_t)(sample_off >> 16);

			// Add end offset
			buffer[off + 4] = (uint16_t)(total_sample_frames);
			buffer[off + 5] = (uint16_t)(total_sample_frames >> 16);

			off += 6;
		}
	}

	for (int i = 0; i < total_frames; i++) {
		float lf = audio_frames[i * 2 + 0];
		float rf = audio_frames[i * 2 + 1];
		lf = Math::clamp(lf, -1.0f, 1.0f);
		rf = Math::clamp(rf, -1.0f, 1.0f);

		int l = (int)(lf * 32768);
		int r = (int)(rf * 32768);
		
		l = l < -32768 ? -32768 : (l > 32767 ? 32767 : l);
		r = r < -32768 ? -32768 : (r > 32767 ? 32767 : r);

		buffer[off + i * 2 + 0] = (uint16_t)l;
		buffer[off + i * 2 + 1] = (uint16_t)r;
	}

	recording.instance();

	recording->set_format(AudioStreamSample::FORMAT_16_BITS);
	recording->set_mix_rate((int)sample_rate);
	recording->set_stereo(true);
	recording->set_data(data);
}

void EngineAudioRecorder::_init() {
	engine_config = Ref<EngineConfig>();
	recording = Ref<AudioStreamSample>();
	min_rpm = 1000;
	top_rpm = 4000;
	duration_per_sample = 1.0f;
	preheat_cycles = 32;
	sample_count = 32;
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
	register_property<EngineAudioRecorder, int>(
		"preheat_cycles", 
		&EngineAudioRecorder::set_preheat_cycles,
		&EngineAudioRecorder::get_preheat_cycles,
		32
	);
	register_property<EngineAudioRecorder, int>(
		"sample_count", 
		&EngineAudioRecorder::set_sample_count,
		&EngineAudioRecorder::get_sample_count,
		32
	);
	register_property<EngineAudioRecorder, bool>(
		"include_audio_header", 
		&EngineAudioRecorder::set_include_audio_header,
		&EngineAudioRecorder::get_include_audio_header,
		true
	);
	
	register_method("record", &EngineAudioRecorder::record);
	register_method("get_recording", &EngineAudioRecorder::get_recording);
}

EngineAudioRecorder::EngineAudioRecorder() {
	
}

EngineAudioRecorder::~EngineAudioRecorder() {
	
}

