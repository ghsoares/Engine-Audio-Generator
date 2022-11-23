#include "engine_audio_player.h"
#include <Math.hpp>

using namespace godot;

void update_sample_buffer(
	PoolByteArray sample_data,
	Vector2 *&buffer,
	uint32_t &buffer_size,
	bool is_16_bit
) {
	PoolByteArray::Read sample_read = sample_data.read();

	if (is_16_bit) {
		uint32_t data_size = sample_data.size() / 2;
		buffer_size = data_size / 2;
		const int16_t *byte_data = (const int16_t *)sample_read.ptr();
		buffer = new Vector2[buffer_size];

		for (uint32_t j = 0; j < buffer_size; j++) {
			float l = byte_data[j * 2 + 0] / (float)(1 << 15);
			float r = byte_data[j * 2 + 1] / (float)(1 << 15);

			buffer[j] = Vector2(l, r);
		}
	} else {
		uint32_t data_size = sample_data.size();
		buffer_size = data_size / 2;
		const int8_t *byte_data = (const int8_t *)sample_read.ptr();
		buffer = new Vector2[buffer_size];

		for (uint32_t j = 0; j < buffer_size; j++) {
			float l = byte_data[j * 2 + 0] / (float)(1 << 7);
			float r = byte_data[j * 2 + 1] / (float)(1 << 7);

			buffer[j] = Vector2(l, r);
		}
	}
}

Vector2 sample_audio(
	Vector2 *audio_buffer,
	uint32_t audio_size,
	float time
) {
	time *= audio_size;
	uint32_t i = (uint32_t)time;
	uint32_t j = i + 1;
	float fract = Math::fmod(time, 1.f);

	Vector2 a = audio_buffer[i % audio_size];
	Vector2 b = audio_buffer[j % audio_size];

	return a * (1 - fract) + b * fract;
}

void EngineAudioPlayer::update_engine_samples() {
	if (engine_samples) {
		delete[] engine_samples;
	}

	if (!engine_stream.is_valid()) {
		engine_samples = nullptr;
		engine_samples_size = 0;
		return;
	}

	ERR_FAIL_COND(engine_stream->get_format() == AudioStreamSample::FORMAT_IMA_ADPCM);

	update_sample_buffer(engine_stream->get_data(), engine_samples, engine_samples_size, engine_stream->get_format() == AudioStreamSample::FORMAT_16_BITS);

	engine_sample_rate = (uint32_t)engine_stream->get_mix_rate();
}

void EngineAudioPlayer::update_combustion_samples() {
	if (combustion_samples) {
		delete[] combustion_samples;
	}

	if (!combustion_stream.is_valid()) {
		combustion_samples = nullptr;
		combustion_samples_size = 0;
		return;
	}

	ERR_FAIL_COND(combustion_stream->get_format() == AudioStreamSample::FORMAT_IMA_ADPCM);

	update_sample_buffer(combustion_stream->get_data(), combustion_samples, combustion_samples_size, combustion_stream->get_format() == AudioStreamSample::FORMAT_16_BITS);

	combustion_sample_rate = (uint32_t)combustion_stream->get_mix_rate();
}

// void EngineAudioPlayer::update_rpm(float delta) {
// 	acc_factor = acceleration_factor;
// 	dec_factor = decceleration_factor;

// 	if (revving) {
// 		acc_factor = 0;
// 	}

// 	float acc = acceleration * acc_factor * delta;
// 	float dec = decceleration * dec_factor * delta;

// 	float dif;

// 	// Decceleration
// 	dif = idle_rpm - current_rpm;
// 	dif = Math::clamp(dif, -dec, dec);
// 	current_rpm += dif;

// 	// Acceleration
// 	dif = max_rpm - current_rpm;
// 	dif = Math::clamp(dif, 0.f, acc);
// 	current_rpm += dif;

// 	if (revving) {
// 		revving = current_rpm > max_rpm - rev_rpm;
// 	} else {
// 		revving = current_rpm >= max_rpm;
// 	}
// }

void EngineAudioPlayer::fill_buffer(float elapsed) {
	uint32_t frames = (uint32_t)playback->get_frames_available();
	if (frames == 0) return;

	uint32_t max_frames = (uint32_t)(elapsed * mix_rate);
	frames = frames < max_frames ? frames : max_frames;

	ERR_FAIL_COND(!generator.is_valid());
	ERR_FAIL_COND(!playback.is_valid());
	ERR_FAIL_COND(!engine_samples);
	ERR_FAIL_COND(!combustion_samples);
	ERR_FAIL_COND(engine_samples_size == 0);
	ERR_FAIL_COND(combustion_samples_size == 0);

	PoolVector2Array buffer;
	buffer.resize(frames);
	PoolVector2Array::Write buf = buffer.write();
	Vector2 *buf_ptr = buf.ptr();

	ERR_FAIL_COND(!playback->can_push_buffer(frames));

	float delta = 1.f / mix_rate;
	float engine_time_spacing = (float)engine_sample_rate / engine_samples_size;
	float combustion_time_spacing = (float)combustion_sample_rate / combustion_samples_size;

	float rpm_lerp = 32000.f * delta;
	float volume_lerp = Math::clamp(256.f * delta, 0.f, 1.f);

	for (uint32_t i = 0; i < frames; i++) {
		// update_rpm(delta);
		current_rpm = current_rpm + Math::clamp(desired_rpm - current_rpm, -rpm_lerp, rpm_lerp);

		float engine_pitch = current_rpm / engine_rpm;
		float combustion_pitch = current_rpm / combustion_rpm;

		Vector2 engine_sound = sample_audio(engine_samples, engine_samples_size, engine_pos);
		Vector2 combustion_sound = sample_audio(combustion_samples, combustion_samples_size, combustion_pos);

		engine_pos = Math::fmod(engine_pos + engine_pitch * engine_time_spacing * delta, 1.f);
		combustion_pos = Math::fmod(combustion_pos + combustion_pitch * combustion_time_spacing * delta, 1.f);

		float engine_vol = engine_volume;
		float combustion_vol = combustion_volume;

		if (revving) {
			if (rev_pos < 0.5f) {
				engine_vol = engine_revving_volume;
				combustion_vol = combustion_revving_volume;
				rev_pos = Math::fmod(rev_pos + delta / rev_outset, 1.f);
			} else {
				rev_pos = Math::fmod(rev_pos + delta / rev_inset, 1.f);
			}
			revving = current_rpm >= max_rpm;
		} else {
			if (current_rpm >= max_rpm) {
				rev_pos = 0;
				revving = true;
			}
		}

		engine_vol = last_engine_volume + (engine_vol - last_engine_volume) * volume_lerp;
		combustion_vol = last_combustion_volume + (combustion_vol - last_combustion_volume) * volume_lerp;

		Vector2 mixed = engine_sound * engine_vol + combustion_sound * combustion_vol;

		buf_ptr[i] = mixed;

		last_engine_volume = engine_vol;
		last_combustion_volume = combustion_vol;
	}

	playback->push_buffer(buffer);
}

void EngineAudioPlayer::process_audio(float delta) {
	if ((float)generator->get_mix_rate() != mix_rate) {
		generator->set_mix_rate(mix_rate);
	}

	fill_buffer((float)delta);
}

void EngineAudioPlayer::_init() {}

void EngineAudioPlayer::_ready() {
	generator = get_stream();
	playback = get_stream_playback();

	if (mix_rate > 0) {
		generator->set_mix_rate(mix_rate);
	}
	fill_buffer(1.f/60.f);
}

EngineAudioPlayer::EngineAudioPlayer() {
	mix_rate = 44100;
	desired_rpm = 1000;
	current_rpm = 1000;

	// idle_rpm = 1000;
	max_rpm = 8000;

	engine_rpm = 1000;
	combustion_rpm = 1000;
	rev_inset = 100;
	rev_outset = 50;

	// acceleration = 8000;
	// decceleration = 2000;
	// acceleration_factor = 0;
	// decceleration_factor = 0;

	engine_volume = 1;
	combustion_volume = 1;

	engine_revving_volume = 0.1f;
	combustion_revving_volume = 0;

	// low_pass_filter_frequency = 128;

	revving = false;

	generator = Ref<AudioStreamGenerator>();
	playback = Ref<AudioStreamPlayback>();
	engine_stream = Ref<AudioStreamSample>();
	combustion_stream = Ref<AudioStreamSample>();

	engine_samples = nullptr;
	combustion_samples = nullptr;
	engine_samples_size = 0;
	combustion_samples_size = 0;

	engine_pos = 0;
	combustion_pos = 0;
	rev_pos = 0;

	// last_mixed = Vector2();

	last_engine_volume = 0;
	last_combustion_volume = 0;
}
EngineAudioPlayer::~EngineAudioPlayer() {
	if (engine_samples) {
		delete[] engine_samples;
	}

	if (combustion_samples) {
		delete[] combustion_samples;
	}
}

void EngineAudioPlayer::_register_methods() {
	register_method("_ready", &EngineAudioPlayer::_ready);
	//register_method("_physics_process", &EngineAudioPlayer::_physics_process);

	register_property<EngineAudioPlayer, Ref<AudioStreamSample>>(
		"engine_stream",
		&EngineAudioPlayer::set_engine_stream,
		&EngineAudioPlayer::get_engine_stream,
		Ref<AudioStreamSample>()
	);
	register_property<EngineAudioPlayer, Ref<AudioStreamSample>>(
		"combustion_stream",
		&EngineAudioPlayer::set_combustion_stream,
		&EngineAudioPlayer::get_combustion_stream,
		Ref<AudioStreamSample>()
	);

	register_property<EngineAudioPlayer, float>(
		"mix_rate",
		&EngineAudioPlayer::set_mix_rate,
		&EngineAudioPlayer::get_mix_rate,
		44100
	);
	register_property<EngineAudioPlayer, float>(
		"rpm",
		&EngineAudioPlayer::set_rpm,
		&EngineAudioPlayer::get_rpm,
		1000
	);
	// register_property<EngineAudioPlayer, float>(
	// 	"idle_rpm",
	// 	&EngineAudioPlayer::set_idle_rpm,
	// 	&EngineAudioPlayer::get_idle_rpm,
	// 	1000
	// );
	register_property<EngineAudioPlayer, float>(
		"max_rpm",
		&EngineAudioPlayer::set_max_rpm,
		&EngineAudioPlayer::get_max_rpm,
		8000
	);
	register_property<EngineAudioPlayer, float>(
		"engine_rpm",
		&EngineAudioPlayer::set_engine_rpm,
		&EngineAudioPlayer::get_engine_rpm,
		1000
	);
	register_property<EngineAudioPlayer, float>(
		"combustion_rpm",
		&EngineAudioPlayer::set_combustion_rpm,
		&EngineAudioPlayer::get_combustion_rpm,
		1000
	);
	register_property<EngineAudioPlayer, float>(
		"rev_inset",
		&EngineAudioPlayer::set_rev_inset,
		&EngineAudioPlayer::get_rev_inset,
		100
	);
	register_property<EngineAudioPlayer, float>(
		"rev_outset",
		&EngineAudioPlayer::set_rev_outset,
		&EngineAudioPlayer::get_rev_outset,
		50
	);
	register_property<EngineAudioPlayer, float>(
		"engine_volume",
		&EngineAudioPlayer::set_engine_volume,
		&EngineAudioPlayer::get_engine_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"combustion_volume",
		&EngineAudioPlayer::set_combustion_volume,
		&EngineAudioPlayer::get_combustion_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"engine_revving_volume",
		&EngineAudioPlayer::set_engine_revving_volume,
		&EngineAudioPlayer::get_engine_revving_volume,
		0.1f
	);
	register_property<EngineAudioPlayer, float>(
		"combustion_revving_volume",
		&EngineAudioPlayer::set_combustion_revving_volume,
		&EngineAudioPlayer::get_combustion_revving_volume,
		0
	);
	// register_property<EngineAudioPlayer, float>(
	// 	"low_pass_filter_frequency",
	// 	&EngineAudioPlayer::set_low_pass_filter_frequency,
	// 	&EngineAudioPlayer::get_low_pass_filter_frequency,
	// 	128
	// );
	

	// register_property<EngineAudioPlayer, float>(
	// 	"acceleration",
	// 	&EngineAudioPlayer::set_acceleration,
	// 	&EngineAudioPlayer::get_acceleration,
	// 	4000
	// );
	// register_property<EngineAudioPlayer, float>(
	// 	"decceleration",
	// 	&EngineAudioPlayer::set_decceleration,
	// 	&EngineAudioPlayer::get_decceleration,
	// 	2000
	// );

	// register_property<EngineAudioPlayer, float>(
	// 	"acceleration_factor",
	// 	&EngineAudioPlayer::set_acceleration_factor,
	// 	&EngineAudioPlayer::get_acceleration_factor,
	// 	0
	// );
	// register_property<EngineAudioPlayer, float>(
	// 	"decceleration_factor",
	// 	&EngineAudioPlayer::set_decceleration_factor,
	// 	&EngineAudioPlayer::get_decceleration_factor,
	// 	0
	// );

	// register_method("is_revving", &EngineAudioPlayer::is_revving);
	register_method("process_audio", &EngineAudioPlayer::process_audio);
}