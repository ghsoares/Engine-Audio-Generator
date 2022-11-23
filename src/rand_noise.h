#ifndef RAND_NOISE_H
#define RAND_NOISE_H

#include <cstdint>
#include <stdlib.h>
#include <time.h>

#define COS_PI4 0.70710678118654752440084436210485
#define COS_PI8 0.92387953251128675612818318939679
#define SIN_PI8 0.3826834323650897717284599840304
#define ISQRT_2 0.70710678118654752440084436210485

static inline int32_t fast_floor(double d) {
	int32_t i = static_cast<int32_t>(d);
	return d < i ? i - 1 : i;
}

class Noise {
private:
	uint32_t seed;
	uint32_t octaves;
	double period;
	double lacunarity;
	double persistence;
	int32_t repeat;

	double fractal_bounding;

	uint32_t hash(uint32_t x, uint32_t y) const {
		x = (x << 16) | (y & 0xffff);

		x ^= x >> 16;
		x *= seed;
		x ^= x >> 16;
		x *= seed;
		x ^= x >> 16;
		return x;
	}

	double grad(uint32_t x_corner, uint32_t y_corner, double x_dist, double y_dist) const {
	// double grad(int32_t x_corner, int32_t y_corner) const {
	// 	double p3x = x_corner * 0.1031;
	// 	double p3y = y_corner * 0.1031;
	// 	double p3z = x_corner * 0.1031;

	// 	p3x -= fast_floor(p3x);
	// 	p3y -= fast_floor(p3y);
	// 	p3z -= fast_floor(p3z);

	// 	double dot = p3x * (p3y + 33.33) + p3y * (p3z + 33.33) + p3z * (p3x + 33.33);
	// 	p3x += dot;
	// 	p3y += dot;
	// 	p3z += dot;

	// 	double val = (p3x + p3y) * p3z;

	// 	return val - fast_floor(val);
		// return hash((uint32_t)x_corner, (uint32_t)y_corner) / 2147483648.0 - 1.0;

		if (repeat > 0) {
			x_corner = (x_corner % repeat + repeat) % repeat;
			y_corner = (y_corner % repeat + repeat) % repeat;
		}
		
		// Select a gradient unit-vector and return
		// its dot product with the distance vector
		switch(hash(x_corner, y_corner) & 0xf)
		{
			// 90 degrees
			case 0x0: return +x_dist;
			case 0x1: return +y_dist;
			case 0x2: return -x_dist;
			case 0x3: return -y_dist;
			// 45 degrees
			case 0x4: return +x_dist*COS_PI4 +y_dist*COS_PI4;
			case 0x5: return -x_dist*COS_PI4 +y_dist*COS_PI4;
			case 0x6: return -x_dist*COS_PI4 -y_dist*COS_PI4;
			case 0x7: return +x_dist*COS_PI4 -y_dist*COS_PI4;
			// 22.5 degrees
			case 0x8: return +x_dist*COS_PI8 +y_dist*SIN_PI8;
			case 0x9: return +x_dist*SIN_PI8 +y_dist*COS_PI8;
			case 0xa: return -x_dist*SIN_PI8 +y_dist*COS_PI8;
			case 0xb: return -x_dist*COS_PI8 +y_dist*SIN_PI8;
			case 0xc: return -x_dist*COS_PI8 -y_dist*SIN_PI8;
			case 0xd: return -x_dist*SIN_PI8 -y_dist*COS_PI8;
			case 0xe: return +x_dist*SIN_PI8 -y_dist*COS_PI8;
			case 0xf: return +x_dist*COS_PI8 -y_dist*SIN_PI8;
			default: return 0.0;
		}
	}

	double fade(double a) const {
		return a*a*a*(a*(a*6.0 - 15.0) + 10.0);
	}

	double lerp(double a, double b, double w) const {
		return (1.0 - w) * a + w * b;
	}

	void calculate_fractal_bounding() {
		double gain = persistence < 0.0 ? -persistence : persistence;
		double amp = gain;
		double amp_fract = 1.0;
		for (uint32_t i = 1; i < octaves; i++) {
			amp_fract += amp;
			amp *= gain;
		}
		fractal_bounding = 1.0 / amp_fract;
	}
public:
	double get(double x, double y) const {
		double mul = fractal_bounding;
		double per = period;
		double n = 0.0;

		for (uint32_t i = 0; i < octaves; i++) {
			double xx = x / per;
			double yy = y / per;

			int32_t ix = fast_floor(xx);
			int32_t iy = fast_floor(yy);

			double fx = xx - ix;
			double fy = yy - iy;

			const double grad00 = grad(ix,		iy,		fx,			fy);
			const double grad10 = grad(ix + 1,	iy,		fx - 1.0,	fy);
			const double grad01 = grad(ix,		iy + 1,	fx,			fy - 1.0);
			const double grad11 = grad(ix + 1,	iy + 1,	fx - 1.0,	fy - 1.0);

			// const double grad00 = grad(ix,		iy);
			// const double grad10 = grad(ix + 1,	iy);
			// const double grad01 = grad(ix,		iy + 1);
			// const double grad11 = grad(ix + 1,	iy + 1);

			// fx = fade(fx);
			// fy = fade(fy);

			n += lerp(
				lerp(grad00, grad10, fx),
				lerp(grad01, grad11, fx),
				fy
			) * ISQRT_2 * mul;

			per /= lacunarity;
			mul *= persistence;
		}

		return n;
	}

	template <typename T>
	T get(T x, T y) const {
		return (T)get((double)x, (double)y);
	}

	void set_seed(uint32_t seed) {
		this->seed = seed;
	}

	void set_octaves(uint32_t octaves) {
		this->octaves = octaves;
		calculate_fractal_bounding();
	}

	void set_lacunarity(double lacunarity) {
		this->lacunarity = lacunarity;
	}

	void set_persistence(double persistence) {
		this->persistence = persistence;
		calculate_fractal_bounding();
	}

	void set_repeat(int32_t repeat) {
		this->repeat = repeat;
	}

	void set_period(double period) {
		this->period = period;
	}

	Noise() {
		srand((uint32_t)time(NULL));
		this->seed = (uint32_t)rand();
		this->octaves = 1;
		this->lacunarity = 2.0;
		this->persistence = 0.5;
		this->repeat = -1;
		this->period = 1.0;
		this->fractal_bounding = 1.0;
		calculate_fractal_bounding();
	}
};

#endif // RAND_NOISE_H