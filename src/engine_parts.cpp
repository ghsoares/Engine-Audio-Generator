#include "engine_parts.h"
#include "engine_utils.h"
#include <Math.hpp>

#define WAVEGUIDE_MAX_AMP 20.0f

void id(uint32_t id, char id_char) {
	for (uint32_t i = 0; i < id; i++) {
		std::cout << id_char;
	}
}

void EngineMain::clear() {
	crankshaft_pos = 0.0;
	noise_pos = 0.0;

	size_t cylinder_count = cylinders.size();
	for (size_t i = 0; i < cylinder_count; i++) {
		cylinders[i]->clear();
	}

	intake_noise_lp->clear();
	vibration_filter->clear();
	muffler->clear();

	crankshaft_fluctuation_lp->clear();
}

void EngineMain::debug_print() {
	// std::cout << "Intake volume: " << intake_volume << std::endl;
	// std::cout << "Exhaust volume: " << exhaust_volume << std::endl;
	// std::cout << "Vibrations volume: " << vibrations_volume << std::endl;
	
	std::cout << "Intake noise factor: " << intake_noise_factor << std::endl;
	std::cout << "Intake valve shift: " << intake_valve_shift << std::endl;
	std::cout << "Exhaust valve shift: " << exhaust_valve_shift << std::endl;
	std::cout << "Crankshaft fluctuation: " << crankshaft_fluctuation << std::endl;
	
	std::cout << "Crankshaft pos: " << crankshaft_pos << std::endl;
	std::cout << "Exhaust collector: " << exhaust_collector << std::endl;
	std::cout << "Intake collector: " << intake_collector << std::endl;

	std::cout << "Number of cylinders: " << cylinders.size() << std::endl;
	for (uint32_t i = 0; i < cylinders.size(); i++) {
		std::cout << "Cylinder N " << i << ": " << std::endl;
		cylinders[i]->debug_print(1);
	}

	std::cout << "Muffler: " << std::endl;
	muffler->debug_print(1);
}

void EngineCylinder::pop(
	float crank_pos, float exhaust_collector, float intake_valve_shift, float exhaust_valve_shift, 
	float &intake, float &exhaust, float &piston_sound, bool &waveguide_dampened
) {
	float crank = godot::Math::fmod(crank_pos + crank_offset, 1.0f);

	cyl_sound = piston_motion(crank) * piston_motion_factor
		+ fuel_ignition(crank, ignition_time) * ignition_factor;

	float ex_valve = exhaust_valve(godot::Math::fmod(crank + exhaust_valve_shift, 1.0f));
	float in_valve = intake_valve(godot::Math::fmod(crank + intake_valve_shift, 1.0f));

	exhaust_waveguide->alpha = exhaust_closed_refl
		+ (exhaust_open_refl - exhaust_closed_refl) * ex_valve;
	intake_waveguide->alpha = intake_closed_refl
		+ (intake_open_refl - intake_closed_refl) * in_valve;
	
	float ex_c1, ex_c0;
	bool ex_dampened;
	exhaust_waveguide->pop(ex_c1, ex_c0, ex_dampened);

	float in_c1, in_c0;
	bool in_dampened;
	intake_waveguide->pop(in_c1, in_c0, in_dampened);

	float ext_c1, ext_c0;
	bool ext_dampened;
	extractor_waveguide->pop(ext_c1, ext_c0, ext_dampened);

	extractor_exhaust = ext_c1;
	extractor_waveguide->push(ex_c0, exhaust_collector);
	
	intake = in_c0;
	exhaust = ext_c0;
	piston_sound = cyl_sound;
	waveguide_dampened = ex_dampened || in_dampened || ext_dampened;
}

void EngineCylinder::push(float intake) {
	float ex_in = (1.0f - std::abs(exhaust_waveguide->alpha)) * cyl_sound * 0.5f;
	exhaust_waveguide->push(ex_in, extractor_exhaust);

	float in_in = (1.0f - std::abs(intake_waveguide->alpha)) * cyl_sound * 0.5f;
	intake_waveguide->push(in_in, intake);
}

void EngineCylinder::clear() {
	exhaust_waveguide->clear();
	intake_waveguide->clear();
	extractor_waveguide->clear();

	cyl_sound = 0;
	extractor_exhaust = 0;
}

void EngineCylinder::debug_print(uint32_t indent) {
	id(indent, ' ');
	std::cout << "Crank offset: " << crank_offset << std::endl;
	
	id(indent, ' ');
	std::cout << "Intake open refl: " << intake_open_refl << std::endl;

	id(indent, ' ');
	std::cout << "Intake closed refl: " << intake_closed_refl << std::endl;

	id(indent, ' ');
	std::cout << "Exhaust open refl: " << exhaust_open_refl << std::endl;

	id(indent, ' ');
	std::cout << "Exhaust closed refl: " << exhaust_closed_refl << std::endl;

	id(indent, ' ');
	std::cout << "Piston motion factor: " << piston_motion_factor << std::endl;

	id(indent, ' ');
	std::cout << "Ignition factor: " << ignition_factor << std::endl;

	id(indent, ' ');
	std::cout << "Ignition time: " << ignition_time << std::endl;

	id(indent, ' ');
	std::cout << "Cyl sound: " << cyl_sound << std::endl;

	id(indent, ' ');
	std::cout << "Extractor exhaust: " << extractor_exhaust << std::endl;

	id(indent, ' ');
	std::cout << "Exhaust waveguide: " << std::endl;
	exhaust_waveguide->debug_print(indent + 1);

	id(indent, ' ');
	std::cout << "Intake waveguide: " << std::endl;
	intake_waveguide->debug_print(indent + 1);

	id(indent, ' ');
	std::cout << "Extractor waveguide: " << std::endl;
	extractor_waveguide->debug_print(indent + 1);
}

void EngineMuffler::clear() {
	straight_pipe->clear();
	size_t muffler_count = muffler_elements.size();
	for (size_t i = 0; i < muffler_count; i++) {
		muffler_elements[i]->clear();
	}
}

void EngineMuffler::debug_print(uint32_t indent) {
	id(indent, ' ');
	std::cout << "Muffler count: " << muffler_elements.size() << std::endl;
	for (uint32_t i = 0; i < muffler_elements.size(); i++) {
		std::cout << "Muffler N " << i << ": " << std::endl;
		muffler_elements[i]->debug_print(1);
	}

	id(indent, ' ');
	std::cout << "Straight pipe: " << std::endl;
	straight_pipe->debug_print(indent + 1);
}

float LowPassFilter::filter(float sample) {
	float ret = (sample - last) * alpha + last;
	last = ret;
	return ret;
}

void LowPassFilter::modify(float freq, uint32_t sample_rate) {
	this->delay = 1.0f / freq;
	this->alpha = ((float)Math_PI * 2 * (1.0f / sample_rate) * freq) /
		((float)Math_PI * 2 * (1.0f / sample_rate) * freq + 1);
}

void LowPassFilter::clear() {
	last = 0;
}

void LoopBuffer::push(float value) {
	data[pos % len] = value;
}

float LoopBuffer::pop() {
	return data[(pos + 1) % len];
}

void LoopBuffer::advance() {
	pos = (pos + 1) % len;
}

void LoopBuffer::modify(uint32_t len, uint32_t sample_rate) {
	this->delay = len / (float)sample_rate;
	float *new_data;

	if (this->data) {
		new_data = new float[len];

		uint32_t min_len = this->len < len ? this->len : len;

		for (uint32_t i = 0; i < min_len; i++) {
			new_data[i] = this->data[i];
		}

		float a = this->data[this->len - 1];
		float b = this->data[0];

		for (uint32_t i = min_len; i < len; i++) {
			float t = (float)i / (float)(len - min_len);
			
			new_data[i] = a + (b - a) * t;
		}

		delete[] this->data;
	} else {
		new_data = new float[len]();
	}

	this->data = new_data;
	this->len = len;
	this->pos = this->pos % len;
}

void LoopBuffer::clear() {
	for (uint32_t i = 0; i < len; i++) {
		data[i] = 0;
	}
	pos = 0;
}

void LoopBuffer::debug_print(uint32_t indent) {
	id(indent, ' ');
	std::cout << "Delay: " << delay << std::endl;

	id(indent, ' ');
	std::cout << "Data length: " << len << std::endl;

	id(indent, ' ');
	std::cout << "Position: " << pos << std::endl;

	/*id(indent, ' ');
	std::cout << "Data: [ ";
	for (uint32_t i = 0; i < len; i++) {
		std::cout << data[i];
	}
	std::cout << " ]" << std::endl;*/
}

void WaveGuide::pop(float &c1, float &c0, bool &dampened) {
	float _c1, _c0;
	bool _c1_dampened, _c0_dampened;
	dampen(chamber1->pop(), _c1, _c1_dampened);
	dampen(chamber0->pop(), _c0, _c0_dampened);

	c1_out = _c1;
	c0_out = _c0;

	c1 = c1_out * (1.0f - std::abs(alpha));
	c0 = c0_out * (1.0f - std::abs(beta));
	dampened = _c1_dampened || _c0_dampened;
}

void WaveGuide::dampen(float sample, float &value, bool &dampened) {
	float sample_abs = std::abs(sample);
	if (sample_abs > WAVEGUIDE_MAX_AMP) {
		value = godot::Math::sign(sample) *
			(-1.0f / (sample_abs - WAVEGUIDE_MAX_AMP + 1.0f) + 1.0f + WAVEGUIDE_MAX_AMP);
		dampened = true;
	} else {
		value = sample;
		dampened = false;
	}
}

void WaveGuide::push(float x0_in, float x1_in) {
	float c0_in = c1_out * alpha + x0_in;
	float c1_in = c0_out * beta + x1_in;

	chamber0->push(c0_in);
	chamber1->push(c1_in);
	chamber0->advance();
	chamber1->advance();
}

void WaveGuide::modify(uint32_t delay, float alpha, float beta, uint32_t sample_rate) {
	if (this->chamber0) {
		this->chamber0->modify(delay, sample_rate);
	} else {
		this->chamber0 = new LoopBuffer(delay, sample_rate);
	}

	if (this->chamber1) {
		this->chamber1->modify(delay, sample_rate);
	} else {
		this->chamber1 = new LoopBuffer(delay, sample_rate);
	}

	this->alpha = alpha;
	this->beta = beta;
}

void WaveGuide::clear() {
	chamber0->clear();
	chamber1->clear();

	c1_out = 0;
	c0_out = 0;
}

void WaveGuide::debug_print(uint32_t indent) {
	id(indent, ' ');
	std::cout << "Alpha: " << alpha << std::endl;

	id(indent, ' ');
	std::cout << "Beta: " << beta << std::endl;

	id(indent, ' ');
	std::cout << "C1 out: " << c1_out << std::endl;

	id(indent, ' ');
	std::cout << "C0 out: " << c0_out << std::endl;

	id(indent, ' ');
	std::cout << "Chamber 0: " << std::endl;
	chamber0->debug_print(indent + 1);

	id(indent, ' ');
	std::cout << "Chamber 1: " << std::endl;
	chamber1->debug_print(indent + 1);
}

EngineMain::EngineMain() {
	// this->intake_volume = 0.0;
	// this->exhaust_volume = 0.0;
	// this->vibrations_volume = 0.0;

	this->cylinders = std::vector<EngineCylinder *>();

	this->intake_noise = nullptr;
	this->intake_noise_factor = 0.0;

	this->intake_noise_lp = nullptr;
	this->vibration_filter = nullptr;
	this->muffler = nullptr;

	this->intake_valve_shift = 0.0;
	this->exhaust_valve_shift = 0.0;
	this->crankshaft_fluctuation = 0.0;

	this->crankshaft_fluctuation_lp = nullptr;
	this->crankshaft_noise = nullptr;

	this->crankshaft_pos = 0.0;
	this->noise_pos = 0.0;
	this->exhaust_collector = 0.0;
	this->intake_collector = 0.0;
}

EngineMain::~EngineMain() {
	for (uint32_t i = 0; i < cylinders.size(); i++) {
		delete this->cylinders[i];
	}

	if (this->intake_noise) {
		delete this->intake_noise;
	}

	if (this->intake_noise_lp) {
		delete this->intake_noise_lp;
	}

	if (this->vibration_filter) {
		delete this->vibration_filter;
	}

	if (this->muffler) {
		delete this->muffler;
	}

	if (this->crankshaft_fluctuation_lp) {
		delete this->crankshaft_fluctuation_lp;
	}

	if (this->crankshaft_noise) {
		delete this->crankshaft_noise;
	}
}

EngineCylinder::EngineCylinder() {
	this->crank_offset = 0.0;
	this->exhaust_waveguide = nullptr;
	this->intake_waveguide = nullptr;
	this->extractor_waveguide = nullptr;

	this->intake_open_refl = 0.0;
	this->intake_closed_refl = 0.0;
	this->exhaust_open_refl = 0.0;
	this->exhaust_closed_refl = 0.0;
	this->piston_motion_factor = 0.0;
	this->ignition_factor = 0.0;
	this->ignition_time = 0.0;
	this->cyl_sound = 0.0;
	this->extractor_exhaust = 0.0;
}

EngineCylinder::~EngineCylinder() {
	if (this->exhaust_waveguide) {
		delete this->exhaust_waveguide;
	}

	if (this->intake_waveguide) {
		delete this->intake_waveguide;
	}

	if (this->extractor_waveguide) {
		delete this->extractor_waveguide;
	}
}

EngineMuffler::EngineMuffler() {
	this->straight_pipe = nullptr;
	this->muffler_elements = std::vector<WaveGuide *>();
}

EngineMuffler::~EngineMuffler() {
	if (this->straight_pipe) {
		delete this->straight_pipe;
	}

	for (uint32_t i = 0; i < muffler_elements.size(); i++) {
		delete this->muffler_elements[i];
	}
}

LowPassFilter::LowPassFilter() {
	this->delay = 0.0;
	this->alpha = 0.0;
	this->last = 0.0;
}

LowPassFilter::LowPassFilter(float freq, uint32_t sample_rate) {
	this->delay = 1.0f / freq;
	this->alpha = ((float)Math_PI * 2 * (1.0f / sample_rate) * freq) /
		((float)Math_PI * 2 * (1.0f / sample_rate) * freq + 1);
	this->last = 0.0;
}

LoopBuffer::LoopBuffer() {
	this->delay = 0.0;
	this->data = nullptr;
	this->len = 0;
	this->pos = 0;
}

LoopBuffer::LoopBuffer(uint32_t len, uint32_t sample_rate) {
	this->delay = len / (float)sample_rate;
	this->data = new float[len]();
	this->len = len;
	this->pos = 0;
}

LoopBuffer::~LoopBuffer() {
	if (this->data) {
		delete[] this->data;
	}
}

WaveGuide::WaveGuide()  {
	this->chamber0 = nullptr;
	this->chamber1 = nullptr;

	this->alpha = 0.0;
	this->beta = 0.0;
	this->c1_out = 0.0;
	this->c0_out = 0.0;
}

WaveGuide::WaveGuide(uint32_t delay, float alpha, float beta, uint32_t sample_rate) {
	this->chamber0 = new LoopBuffer(delay, sample_rate);
	this->chamber1 = new LoopBuffer(delay, sample_rate);
	this->alpha = alpha;
	this->beta = beta;

	this->c1_out = 0.0;
	this->c0_out = 0.0;
}

WaveGuide::~WaveGuide()  {
	if (this->chamber0) {
		delete this->chamber0;
	}
	if (this->chamber1) {
		delete this->chamber1;
	}
}