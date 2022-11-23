#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <Math.hpp>
#include <cstdint>

#define SPEED_OF_SOUND 343.0f
#define PI2F (2.0f * (float)Math_PI)
#define PI4F (4.0f * (float)Math_PI)

inline float exhaust_valve(float crank_pos) {
	if (0.75 < crank_pos && crank_pos < 1.0) {
		return -godot::Math::sin(crank_pos * PI4F);
	}
	return 0.0;
}

inline float intake_valve(float crank_pos) {
	if (0.0 < crank_pos && crank_pos < 0.25) {
		return godot::Math::sin(crank_pos * PI4F);
	}
	return 0.f;
}

inline float fuel_ignition(float crank_pos, float timing) {
	if (0.5 < crank_pos && crank_pos < timing * 0.5 + 0.5) {
		return godot::Math::sin(PI2F * ((crank_pos - 0.5f) / timing));
	}
	return 0.f;
}

inline float piston_motion(float crank_pos) {
	return godot::Math::cos(crank_pos * PI4F);
}

inline uint32_t seconds_to_samples(float seconds, uint32_t sample_rate) {
	uint32_t samples = (uint32_t)(seconds * sample_rate);
	return samples > 1 ? samples : 1;
}

inline uint32_t distance_to_samples(float meters, uint32_t sample_rate) {
	return seconds_to_samples(meters / SPEED_OF_SOUND, sample_rate);
}

#endif // ENGINE_UTILS_H