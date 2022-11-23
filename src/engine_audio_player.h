#ifndef ENGINE_AUDIO_PLAYER_H
#define ENGINE_AUDIO_PLAYER_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <Ref.hpp>
#include <AudioStreamPlayer.hpp>
#include <AudioStreamSample.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>

namespace godot {

class EngineAudioPlayer : public AudioStreamPlayer {
	GODOT_CLASS(EngineAudioPlayer, AudioStreamPlayer)
private:
	float mix_rate;
	float desired_rpm;
	float current_rpm;
	float max_rpm;

	// float idle_rpm;
	// float max_rpm;
	// float rev_rpm;

	float engine_rpm;
	float combustion_rpm;
	float rev_inset;
	float rev_outset;

	float engine_volume;
	float combustion_volume;
	float engine_revving_volume;
	float combustion_revving_volume;

	// float acceleration;
	// float decceleration;
	// float acceleration_factor;
	// float decceleration_factor;

	// float acc_factor;
	// float dec_factor;

	bool revving;

	Ref<AudioStreamGenerator> generator;
	Ref<AudioStreamGeneratorPlayback> playback;
	Ref<AudioStreamSample> engine_stream;
	Ref<AudioStreamSample> combustion_stream;

	Vector2 *engine_samples;
	Vector2 *combustion_samples;
	uint32_t engine_samples_size;
	uint32_t combustion_samples_size;
	uint32_t engine_sample_rate;
	uint32_t combustion_sample_rate;

	float engine_pos;
	float combustion_pos;
	float rev_pos;

	float last_engine_volume;
	float last_combustion_volume;

	// Vector2 last_mixed;

	void update_engine_samples();
	void update_combustion_samples();
	// void update_rpm(float delta);
	void fill_buffer(float elapsed);
public:
	static void _register_methods();

	void set_engine_stream(Ref<AudioStreamSample> p_stream) {
		engine_stream = p_stream;
		update_engine_samples();
	}
	Ref<AudioStreamSample> get_engine_stream() const {return engine_stream;}

	void set_combustion_stream(Ref<AudioStreamSample> p_stream) {
		combustion_stream = p_stream;
		update_combustion_samples();
	}
	Ref<AudioStreamSample> get_combustion_stream() const {return combustion_stream;}

	void set_mix_rate(float p_rate) {mix_rate = p_rate;}
	float get_mix_rate() const {return mix_rate;}

	void set_rpm(float p_rpm) {desired_rpm = p_rpm;}
	float get_rpm() const {return desired_rpm;}

	void set_max_rpm(float p_max_rpm) {max_rpm = p_max_rpm;}
	float get_max_rpm() const {return max_rpm;}

	// void set_idle_rpm(float p_rpm) {idle_rpm = p_rpm;}
	// float get_idle_rpm() const {return idle_rpm;}

	// void set_max_rpm(float p_rpm) {max_rpm = p_rpm;}
	// float get_max_rpm() const {return max_rpm;}

	void set_engine_rpm(float p_rpm) {engine_rpm = p_rpm;}
	float get_engine_rpm() const {return engine_rpm;}

	void set_combustion_rpm(float p_rpm) {combustion_rpm = p_rpm;}
	float get_combustion_rpm() const {return combustion_rpm;}

	void set_rev_inset(float p_inset) {rev_inset = p_inset;}
	float get_rev_inset() const {return rev_inset;}

	void set_rev_outset(float p_out) {rev_outset = p_out;}
	float get_rev_outset() const {return rev_outset;}

	void set_engine_volume(float p_volume) {engine_volume = p_volume;}
	float get_engine_volume() const {return engine_volume;}

	void set_combustion_volume(float p_volume) {combustion_volume = p_volume;}
	float get_combustion_volume() const {return combustion_volume;}

	void set_engine_revving_volume(float p_volume) {engine_revving_volume = p_volume;}
	float get_engine_revving_volume() const {return engine_revving_volume;}

	void set_combustion_revving_volume(float p_volume) {combustion_revving_volume = p_volume;}
	float get_combustion_revving_volume() const {return combustion_revving_volume;}

	// void set_low_pass_filter_frequency(float p_frequency) {low_pass_filter_frequency = p_frequency;}
	// float get_low_pass_filter_frequency() const {return low_pass_filter_frequency;}

	// void set_acceleration(float p_acceleration) {acceleration = p_acceleration;}
	// float get_acceleration() const {return acceleration;}

	// void set_decceleration(float p_decceleration) {decceleration = p_decceleration;}
	// float get_decceleration() const {return decceleration;}

	// void set_acceleration_factor(float p_factor) {acceleration_factor = p_factor;}
	// float get_acceleration_factor() const {return acceleration_factor;}

	// void set_decceleration_factor(float p_factor) {decceleration_factor = p_factor;}
	// float get_decceleration_factor() const {return decceleration_factor;}

	// bool is_revving() const {return revving;}
	void process_audio(float delta);

	void _ready();
	void _init();

	EngineAudioPlayer();
	~EngineAudioPlayer();
};

}

#endif // ENGINE_AUDIO_PLAYER_H