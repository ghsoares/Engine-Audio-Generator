#ifndef CAR_ENGINE_H
#define CAR_ENGINE_H

#include <vector>
#include "rand_xorshift.h"
#include <stdio.h>
#include <iostream>

class EngineMain;
class EngineCylinder;
class EngineMuffler;
class LowPassFilter;
class WaveGuide;
class LoopBuffer;
class DelayLine;

class EngineMain {
public:
	// float intake_volume;
	// float exhaust_volume;
	// float vibrations_volume;

	std::vector<EngineCylinder *> cylinders;
	Noise *intake_noise;
	float intake_noise_factor;
	LowPassFilter *intake_noise_lp;
	LowPassFilter *vibration_filter;
	EngineMuffler *muffler;

	float intake_valve_shift;
	float exhaust_valve_shift;
	float crankshaft_fluctuation;

	LowPassFilter *crankshaft_fluctuation_lp;
	Noise *crankshaft_noise;

	float crankshaft_pos;
	float noise_pos;
	float exhaust_collector;
	float intake_collector;

	void clear();

	void debug_print();

	EngineMain();
	~EngineMain();
};

class EngineCylinder {
public:
	float crank_offset;
	WaveGuide *exhaust_waveguide;
	WaveGuide *intake_waveguide;
	WaveGuide *extractor_waveguide;

	float intake_open_refl;
	float intake_closed_refl;
	float exhaust_open_refl;
	float exhaust_closed_refl;

	float piston_motion_factor;
	float ignition_factor;
	float ignition_time;

	float cyl_sound;
	float extractor_exhaust;

	void pop(
		float crank_pos, float exhaust_collector, float intake_valve_shift, float exhaust_valve_shift, 
		float &intake, float &exhaust, float &piston_ignition, bool &waveguide_dampened
	);
	void push(float intake);

	void clear();

	void debug_print(uint32_t indent);

	EngineCylinder();
	~EngineCylinder();
};

class EngineMuffler {
public:
	WaveGuide *straight_pipe;
	std::vector<WaveGuide *> muffler_elements;

	void clear();

	void debug_print(uint32_t indent);

	EngineMuffler();
	~EngineMuffler();
};

class LowPassFilter {
public:
	float delay;
	float alpha;
	float last;

	float filter(float sample);

	float get_frequency() const {return 1.f / delay;}

	void modify(float freq, uint32_t sample_rate);

	void clear();

	LowPassFilter();
	LowPassFilter(float freq, uint32_t sample_rate);
	~LowPassFilter() {}
};

class LoopBuffer {
public:
	float delay;
	float *data;
	uint32_t len;
	int pos;

	void push(float value);
	float pop();
	void advance();

	void modify(uint32_t len, uint32_t sample_rate);

	void clear();

	void debug_print(uint32_t indent);

	LoopBuffer();
	LoopBuffer(uint32_t len, uint32_t sample_rate);
	~LoopBuffer();
};

class WaveGuide {
public:
	LoopBuffer *chamber0;
	LoopBuffer *chamber1;

	float alpha;
	float beta;

	float c1_out;
	float c0_out;
	
	void pop(float &c1, float &c0, bool &dampened);
	void dampen(float sample, float &value, bool &dampened);
	void push(float x0_in, float x1_in);

	void modify(uint32_t delay, float alpha, float beta, uint32_t sample_rate);

	void clear();

	void debug_print(uint32_t indent);

	WaveGuide();
	WaveGuide(uint32_t delay, float alpha, float beta, uint32_t sample_rate);
	~WaveGuide();
};

#endif // CAR_ENGINE_H