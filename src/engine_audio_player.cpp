#include "engine_audio_player.h"
#include <Math.hpp>

using namespace godot;

void EngineAudioPlayer::update_channel(EngineAudioChannel *channel, Ref<AudioStreamSample> stream) {
	bool channel_valid = false;

	int start_off = 0;
	
	if (stream.is_valid()) {
		// Supports only 16 bit pcm stereo file
		if (stream->get_format() == AudioStreamSample::FORMAT_16_BITS && stream->is_stereo()) {
			PoolByteArray data = stream->get_data();
			if (data.size() > 4) {
				// Read data
				PoolByteArray::Read data_read = data.read();
				uint16_t *buffer = (uint16_t *)data_read.ptr();
				// Identify as engine audio file
				if (buffer[0] == 0x5555 && buffer[1] == 0xAAAA) {
					// Get version
					uint32_t version = (uint32_t)(buffer[2]) | ((uint32_t)(buffer[3]) << 16);
					if (version == 0) {
						// Get data size
						uint32_t data_size = (uint32_t)(buffer[4]) | ((uint32_t)(buffer[5]) << 16);

						// Get number of samples
						uint32_t sample_count = (uint32_t)(buffer[6]) | ((uint32_t)(buffer[7]) << 16);

						// Get padding frames
						uint32_t padding_frames = (uint32_t)(buffer[8]) | ((uint32_t)(buffer[9]) << 16);

						// Move buffer
						buffer = buffer + 10;
						start_off += 5;

						// Construct array of frames
						if (channel->frames) {
							delete[] channel->frames;
						}
						channel->frames = new Vector2[data_size / 4];
						channel->frame_count = data_size / 4;
						
						// Construct array of samples
						if (channel->samples) {
							delete[] channel->samples;
						}
						channel->samples = new EngineAudioSample[sample_count]();
						channel->sample_count = sample_count;

						// Get channel sample rate
						channel->sample_rate = (float)stream->get_mix_rate();

						// For each sample
						for (uint32_t i = 0; i < sample_count; i++) {
							// Get rpm
							uint32_t rpmi = (uint32_t)(buffer[0]) | ((uint32_t)(buffer[1]) << 16);
							float rpm;
							memcpy(&rpm, &rpmi, sizeof(uint32_t));

							// Get start offset
							int32_t start = (int32_t)(buffer[2]) | ((int32_t)(buffer[3]) << 16);

							// Get end offset
							int32_t end = (int32_t)(buffer[4]) | ((int32_t)(buffer[5]) << 16);

							// Get the sample
							EngineAudioSample *sample = &channel->samples[i];

							// Set the variables
							sample->rpm = rpm;
							sample->start = start;
							sample->end = end;
							sample->sample_rate_ratio = channel->sample_rate / (float)(end - start);

							// Offset the buffer
							buffer = buffer + 6;
							start_off += 3;
						}

						// Skip padding frames
						buffer = buffer + (padding_frames * 2);
						start_off += padding_frames;

						// For each frame
						for (uint32_t i = 0; i < data_size / 4; i++) {
							// Get the data
							float l = (int16_t)buffer[0] / (float)(1 << 15);
							float r = (int16_t)buffer[1] / (float)(1 << 15);

							// Set the frame
							channel->frames[i] = Vector2(l, r);
							
							buffer = buffer + 2;
						}

						channel_valid = true;
					} else {
						WARN_PRINT("Invalid engine audio file version");
					}
				} else {
					WARN_PRINT("Invalid engine audio file identifier");
				}
			} else {
				WARN_PRINT("Engine audio file too small");
			}
		} else {
			WARN_PRINT("Engine audio file only supports 16 bit PCM stereo data");
		}
	}

	if (!channel_valid) {
		if (channel->frames) delete[] channel->frames;
		if (channel->samples) delete[] channel->samples;
		channel->frames = nullptr;
		channel->samples = nullptr;
		channel->frame_count = 0;
		channel->sample_count = 0;
		channel->sample_rate = 44100;
		WARN_PRINT("Invalid engine audio file");
	}	
	
	channel->dirty = false;

	// Godot::print("----------------------------");
	// Godot::print(
	// 	"frame_count:{0}\nsample_count:{1}\nsample_rate:{2}", 
	// 	channel->frame_count, 
	// 	channel->sample_count, 
	// 	channel->sample_rate
	// );
	// for (int i = 0; i < channel->sample_count; i++) {
	// 	EngineAudioSample *sample = &channel->samples[i];

	// 	Godot::print(
	// 		"rpm:{0}\nstart:{1}\nend:{2}\nsample_rate_ratio:{3}", 
	// 		sample->rpm,
	// 		sample->start + start_off,
	// 		sample->end + start_off,
	// 		sample->sample_rate_ratio
	// 	);
	// }
	// Godot::print("----------------------------");
}

void EngineAudioPlayer::update_dirty_channels() {
	if (crankshaft_channel->dirty) update_channel(crankshaft_channel, crankshaft_stream);
	if (ignition_channel->dirty) update_channel(ignition_channel, ignition_stream);
	if (exhaust_channel->dirty) update_channel(exhaust_channel, exhaust_stream);
}

void EngineAudioPlayer::process_audio(float delta) {
	update_dirty_channels();

	ERR_FAIL_COND(!generator.is_valid());
	ERR_FAIL_COND(!generator_playback.is_valid());

	float mix_rate = generator->get_mix_rate();

	uint32_t frames = (uint32_t)generator_playback->get_frames_available();
	if (frames == 0) return;

	uint32_t max_frames = (uint32_t)(delta * mix_rate);
	frames = frames < max_frames ? frames : max_frames;

	ERR_FAIL_COND(!generator_playback->can_push_buffer(frames));

	delta = 1.0f / mix_rate;

	PoolVector2Array buffer;
	buffer.resize(frames);
	PoolVector2Array::Write buf = buffer.write();
	Vector2 *buf_ptr = buf.ptr();

	float volf = volume_blend >= 0 ? volume_blend * delta : -1;
	float rpmf = rpm_blend >= 0 ? rpm_blend * delta : -1;

	// crankshaft_channel->print_info(rpm);

	for (uint32_t i = 0; i < frames; i++) {
		internal_rpm += rpmf >= 0 ? Math::clamp(
			rpm - internal_rpm, -rpmf, rpmf
		) : rpm - internal_rpm;

		crankshaft_channel->advance(internal_rpm, delta);
		ignition_channel->advance(internal_rpm, delta);
		exhaust_channel->advance(internal_rpm, delta);

		internal_master_volume += volf >= 0 ? Math::clamp(
			master_volume - internal_master_volume, -volf, volf
		) : master_volume - internal_master_volume;
		internal_crankshaft_volume += volf >= 0 ? Math::clamp(
			crankshaft_volume - internal_crankshaft_volume, -volf, volf
		) : crankshaft_volume - internal_crankshaft_volume;
		internal_ignition_volume += volf >= 0 ? Math::clamp(
			ignition_volume - internal_ignition_volume, -volf, volf
		) : ignition_volume - internal_ignition_volume;
		internal_exhaust_volume += volf >= 0 ? Math::clamp(
			exhaust_volume - internal_exhaust_volume, -volf, volf
		) : exhaust_volume - internal_exhaust_volume;

		Vector2 crankshaft = crankshaft_channel->get_sample(internal_rpm) * internal_crankshaft_volume;
		Vector2 ignition = ignition_channel->get_sample(internal_rpm) * internal_ignition_volume;
		Vector2 exhaust = exhaust_channel->get_sample(internal_rpm) * internal_exhaust_volume;

		Vector2 mixed = (crankshaft + ignition + exhaust) * internal_master_volume;

		buf_ptr[i] = mixed;
	}

	generator_playback->push_buffer(buffer);
}

void EngineAudioPlayer::_init() {}

EngineAudioPlayer::EngineAudioPlayer() {
	generator = Ref<AudioStreamGenerator>();
	generator_playback = Ref<AudioStreamGeneratorPlayback>();
	crankshaft_stream = Ref<AudioStreamSample>();
	ignition_stream = Ref<AudioStreamSample>();
	exhaust_stream = Ref<AudioStreamSample>();

	crankshaft_channel = new EngineAudioChannel();
	ignition_channel = new EngineAudioChannel();
	exhaust_channel = new EngineAudioChannel();

	rpm = 1000;
	master_volume = 1;
	crankshaft_volume = 1;
	ignition_volume = 1;
	exhaust_volume = 1;

	internal_rpm = 1000;
	internal_master_volume = 1;
	internal_crankshaft_volume = 1;
	internal_ignition_volume = 1;
	internal_exhaust_volume = 1;

	volume_blend = -1;
	rpm_blend = -1;
}

EngineAudioPlayer::~EngineAudioPlayer() {
	if (crankshaft_channel) {
		delete crankshaft_channel;
	}
	if (ignition_channel) {
		delete ignition_channel;
	}
	if (exhaust_channel) {
		delete exhaust_channel;
	}
}

void EngineAudioPlayer::_register_methods() {
	register_property<EngineAudioPlayer, Ref<AudioStreamGenerator>>(
		"audio_generator",
		&EngineAudioPlayer::set_audio_generator,
		&EngineAudioPlayer::get_audio_generator,
		Ref<AudioStreamGenerator>()
	);
	register_property<EngineAudioPlayer, Ref<AudioStreamGeneratorPlayback>>(
		"audio_generator_playback",
		&EngineAudioPlayer::set_audio_generator_playback,
		&EngineAudioPlayer::get_audio_generator_playback,
		Ref<AudioStreamGeneratorPlayback>()
	);
	register_property<EngineAudioPlayer, Ref<AudioStreamSample>>(
		"crankshaft_stream",
		&EngineAudioPlayer::set_crankshaft_stream,
		&EngineAudioPlayer::get_crankshaft_stream,
		Ref<AudioStreamSample>()
	);
	register_property<EngineAudioPlayer, Ref<AudioStreamSample>>(
		"ignition_stream",
		&EngineAudioPlayer::set_ignition_stream,
		&EngineAudioPlayer::get_ignition_stream,
		Ref<AudioStreamSample>()
	);
	register_property<EngineAudioPlayer, Ref<AudioStreamSample>>(
		"exhaust_stream",
		&EngineAudioPlayer::set_exhaust_stream,
		&EngineAudioPlayer::get_exhaust_stream,
		Ref<AudioStreamSample>()
	);
	register_property<EngineAudioPlayer, float>(
		"rpm",
		&EngineAudioPlayer::set_rpm,
		&EngineAudioPlayer::get_rpm,
		1000
	);
	register_property<EngineAudioPlayer, float>(
		"master_volume",
		&EngineAudioPlayer::set_master_volume,
		&EngineAudioPlayer::get_master_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"crankshaft_volume",
		&EngineAudioPlayer::set_crankshaft_volume,
		&EngineAudioPlayer::get_crankshaft_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"ignition_volume",
		&EngineAudioPlayer::set_ignition_volume,
		&EngineAudioPlayer::get_ignition_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"exhaust_volume",
		&EngineAudioPlayer::set_exhaust_volume,
		&EngineAudioPlayer::get_exhaust_volume,
		1
	);
	register_property<EngineAudioPlayer, float>(
		"rpm_blend",
		&EngineAudioPlayer::set_rpm_blend,
		&EngineAudioPlayer::get_rpm_blend,
		-1
	);
	register_property<EngineAudioPlayer, float>(
		"volume_blend",
		&EngineAudioPlayer::set_volume_blend,
		&EngineAudioPlayer::get_volume_blend,
		-1
	);
	
	register_method("process_audio", &EngineAudioPlayer::process_audio);
}