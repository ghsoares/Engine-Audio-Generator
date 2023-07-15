#ifndef UNITS_H
#define UNITS_H

#include "units.h"

namespace math {
	// Expressions
	template<typename T>
	inline constexpr T min(T a, T b) {
		return a < b ? a : b;
	}

	template<typename T>
	inline constexpr T max(T a, T b) {
		return a > b ? a : b;
	}

	template<typename T>
	inline constexpr T clamp(T x, T a, T b) {
		return x < a ? a : (x > b ? b : x);
	}

	template<typename T>
	inline constexpr T abs(T x) {
		return x < 0 ? -x : x;
	}

    inline constexpr float sqrt(float x) {
        return std::sqrtf(x);
    }

    inline constexpr float pow(float x, float exp) {
        return std::powf(x, exp);  
    }

	// Conversions
    inline constexpr float distance(float v, float unit) {
        return v * unit;
    }

    inline constexpr float area(float v, float unit) {
        return v * unit;
    }

    inline constexpr float torque(float v, float unit) {
        return v * unit;
    }

    inline constexpr float rpm(float rpm) {
        return rpm * 0.104719755f;
    }

    inline constexpr float to_rpm(float rad_s) {
        return rad_s / 0.104719755f;
    }

    inline constexpr float pressure(float v, float unit) {
        return v * unit;
    }

    inline constexpr float psia(float p) {
        return units::pressure(p, units::psig) - units::pressure(1, units::atm);
    }

    inline constexpr float to_psia(float p) {
        return (p + units::pressure(1, units::atm)) / units::psig;
    }

    inline constexpr float mass(float v, float unit) {
        return v * unit;
    }

    inline constexpr float force(float v, float unit) {
        return v * unit;
    }

    inline constexpr float volume(float v, float unit) {
        return v * unit;
    }

    inline constexpr float flow(float v, float unit) {
        return v * unit;
    }

    inline constexpr float convert(float v, float unit0, float unit1) {
        return v * (unit0 / unit1);
    }

    inline constexpr float convert(float v, float unit) {
        return v / unit;
    }

    inline constexpr float celcius(float T_C) {
        return T_C * C + K0;
    }

    inline constexpr float kelvin(float T) {
        return T * K;
    }

    inline constexpr float fahrenheit(float T_F) {
        return F * (T_F - F0);
    }

    inline constexpr float to_absolute_fahrenheit(float T) {
        return T / F;
    }

    inline constexpr float angle(float v, float unit) {
        return v * unit;
    }

    inline constexpr float energy(float v, float unit) {
        return v * unit;
    }
}

#endif