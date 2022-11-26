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

static inline float fposmod(float a, float b) {
	return Math::fmod(Math::fmod(a, b) + b, b);
}

class EngineAudioPlayer : public Reference {
	GODOT_CLASS(EngineAudioPlayer, Reference)
public:
	class EngineAudioSample {
	public:
		float rpm;
		int start;
		int end;
		float sample_rate_ratio;
		float pos;

		Vector2 get_sample(Vector2 *frames) {
			int size = (end - start);
			float t = pos * size;
			uint32_t i = (uint32_t)t;
			uint32_t j = i + 1;
			float fract = Math::fmod(t, 1.0f);

			Vector2 a = frames[start + i % size];
			Vector2 b = frames[start + j % size];

			return a * (1 - fract) + b * fract;
		}

		EngineAudioSample() {
			rpm = 1;
			start = 0;
			end = 1;
			sample_rate_ratio = 0;
			pos = 0;
		}
		~EngineAudioSample() {}
	};

	class EngineAudioChannel {
	public:
		Vector2 *frames;
		EngineAudioSample *samples;
		int frame_count;
		int sample_count;
		float sample_rate;
		bool dirty;

		void advance(float rpm, float delta) {
			for (int i = 0; i < sample_count; i++) {
				EngineAudioSample *sample = &samples[i];
				float pitch = rpm / sample->rpm;

				sample->pos = fposmod(sample->pos + sample->sample_rate_ratio * delta * pitch, 1.0f);
			}
		}

		void set_pos(float rpm, float secs) {
			for (int i = 0; i < sample_count; i++) {
				EngineAudioSample *sample = &samples[i];
				float pitch = rpm / sample->rpm;

				sample->pos = fposmod(sample->sample_rate_ratio * secs * pitch, 1.0f);
			}
		}

		Vector2 get_sample(float rpm) {
			if (sample_count == 0) return Vector2();

			if (sample_count == 1) {
				EngineAudioSample *sample = &samples[0];
				return sample->get_sample(frames);
			}

			for (int i = 0; i < sample_count - 1; i++) {
				EngineAudioSample *sample0 = &samples[i];
				EngineAudioSample *sample1 = &samples[i + 1];

				if (i < sample_count - 2 && sample1->rpm < rpm) continue;
				if (i > 0 && sample0->rpm > rpm) continue;

				float st = (rpm - sample0->rpm) / (sample1->rpm - sample0->rpm);
				st = st < 0 ? 0 : (st > 1 ? 1 : st);

				Vector2 a = sample0->get_sample(frames);
				Vector2 b = sample1->get_sample(frames);

				return a * (1 - st) + b * st;
			}

			return Vector2();
		}

		void print_info(float rpm) {
			if (sample_count > 1) {
				Godot::print("RPM: {0}", rpm);
				for (int i = 0; i < sample_count - 1; i++) {
					EngineAudioSample *sample0 = &samples[i];
					EngineAudioSample *sample1 = &samples[i + 1];

					if (i < sample_count - 2 && sample1->rpm < rpm) continue;
					if (i > 0 && sample0->rpm > rpm) continue;

					float st = (rpm - sample0->rpm) / (sample1->rpm - sample0->rpm);
					st = st < 0 ? 0 : (st > 1 ? 1 : st);

					Godot::print("Idx0: {0}, Idx1: {1}, Rpm1: {2}, Rpm2: {3}, Factor: {4}", i, i + 1, sample0->rpm, sample1->rpm, st);

					break;
				}
			}
		}
		
		EngineAudioChannel() {
			frames = nullptr;
			samples = nullptr;
			frame_count = 0;
			sample_count = 0;
			sample_rate = 44100;
			dirty = true;
		}
		~EngineAudioChannel() {
			if (frames) delete[] frames;
			if (samples) delete[] samples;
		}
	};

	Ref<AudioStreamGenerator> generator;
	Ref<AudioStreamGeneratorPlayback> generator_playback;
	Ref<AudioStreamSample> crankshaft_stream;
	Ref<AudioStreamSample> ignition_stream;
	Ref<AudioStreamSample> exhaust_stream;

	EngineAudioChannel *crankshaft_channel;
	EngineAudioChannel *ignition_channel;
	EngineAudioChannel *exhaust_channel;

	float rpm;
	float master_volume;
	float crankshaft_volume;
	float ignition_volume;
	float exhaust_volume;

	float rpm_blend;
	float volume_blend;

	float internal_rpm;
	float internal_master_volume;
	float internal_crankshaft_volume;
	float internal_ignition_volume;
	float internal_exhaust_volume;

	void update_channel(EngineAudioChannel *channel, Ref<AudioStreamSample> stream);
	void update_dirty_channels();
public:
	static void _register_methods();

	void set_audio_generator(Ref<AudioStreamGenerator> p_generator) {
		generator = p_generator;
	}
	Ref<AudioStreamGenerator> get_audio_generator() const {return generator;}

	void set_audio_generator_playback(Ref<AudioStreamGeneratorPlayback> p_playback) {
		generator_playback = p_playback;
	}
	Ref<AudioStreamGeneratorPlayback> get_audio_generator_playback() const {return generator_playback;}

	void set_crankshaft_stream(Ref<AudioStreamSample> p_stream) {
		crankshaft_stream = p_stream;
		crankshaft_channel->dirty = true;
	}
	Ref<AudioStreamSample> get_crankshaft_stream() const {return crankshaft_stream;}
	
	void set_ignition_stream(Ref<AudioStreamSample> p_stream) {
		ignition_stream = p_stream;
		ignition_channel->dirty = true;
	}
	Ref<AudioStreamSample> get_ignition_stream() const {return ignition_stream;}

	void set_exhaust_stream(Ref<AudioStreamSample> p_stream) {
		exhaust_stream = p_stream;
		exhaust_channel->dirty = true;
	}
	Ref<AudioStreamSample> get_exhaust_stream() const {return exhaust_stream;}

	void set_rpm(float p_volume) {rpm = p_volume;}
	float get_rpm() const {return rpm;}

	void set_master_volume(float p_volume) {master_volume = p_volume;}
	float get_master_volume() const {return master_volume;}

	void set_crankshaft_volume(float p_volume) {crankshaft_volume = p_volume;}
	float get_crankshaft_volume() const {return crankshaft_volume;}

	void set_ignition_volume(float p_volume) {ignition_volume = p_volume;}
	float get_ignition_volume() const {return ignition_volume;}

	void set_exhaust_volume(float p_volume) {exhaust_volume = p_volume;}
	float get_exhaust_volume() const {return exhaust_volume;}

	void set_rpm_blend(float p_blend) {rpm_blend = p_blend;}
	float get_rpm_blend() const {return rpm_blend;}

	void set_volume_blend(float p_blend) {volume_blend = p_blend;}
	float get_volume_blend() const {return volume_blend;}

	void process_audio(float delta);
	void _init();

	EngineAudioPlayer();
	~EngineAudioPlayer();
};

}

#endif // ENGINE_AUDIO_PLAYER_H