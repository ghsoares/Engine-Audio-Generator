#ifndef RAND_XORSHIFT
#define RAND_XORSHIFT

#include <cstdint>
#include <stdlib.h>
#include <time.h>

class XorShiftRandom {
protected:
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
public:
	uint32_t next_u32() {
		uint32_t t = x ^ (x << 11);
		x = y; 
		y = z; 
		z = w;
		uint32_t _w = w;
		w = _w ^ (_w >> 19) ^ (t ^ (t >> 8));
		return w;
	}

	double next_f64() {
		return next_u32() / 2147483648.0 - 1.0;
	}

	float next_f32() {
		return (float)next_f64();
	}

	XorShiftRandom() {
		x = 123456789;
		y = 362436069;
		z = 521288629;
		w = 88675123;
	}
};

class Noise : public XorShiftRandom {
public:
	void set_random_seed() {
		srand((uint32_t)time(NULL));
		x = (uint32_t)rand();
		y = (uint32_t)rand();
		z = (uint32_t)rand();
		w = (uint32_t)rand();
	}

	void set_seed(uint32_t seed) {
		srand(seed);
		x = (uint32_t)rand();
		y = (uint32_t)rand();
		z = (uint32_t)rand();
		w = (uint32_t)rand();
	}

	Noise() {
		set_random_seed();
	}

	Noise(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3) {
		x = seed0;
		y = seed1;
		z = seed2;
		w = seed3;
	}
};

#endif // RAND_XORSHIFT