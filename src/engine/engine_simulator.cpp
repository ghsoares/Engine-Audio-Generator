#include "engine_simulator.h"
#include <VisualServer.hpp>

using namespace godot;

void EngineSimulator::simulate(float p_dt, int p_frames) {
	if (!engine_starter.is_valid()) return;

	// Set the starter
	engine_starter->starter = starter_factor;

	// Run n steps
	for (int i = 0; i < p_frames; i++) {
		// Simulate the starter
		engine_starter->step(p_dt);

		// Record the engine starter
		engine_starter->record_audio_sample(microphone);
		// audio_write.set(Math::sin(time * (float)Math_TAU));

		// Advance audio buffer
		audio_write.move();

		time += p_dt;
	}
}

void EngineSimulator::draw_simulation(const RID p_canvas_item) {
	if (!engine_starter.is_valid()) return;

	// The draw color
	Color color = Color(1.0f, 1.0f, 1.0f, 0.2f);

	// Current updated gear
	Ref<EngineGear> gear = engine_starter->main_gear;

	// Get visual server
	VisualServer *vs = VisualServer::get_singleton();

	// For each gear
	while (gear.is_valid()) {
		// Get the gear position
		Vector2 gear_pos = gear->position + gear->gcom;

		// Reset the draw transform
		vs->canvas_item_add_set_transform(p_canvas_item, Transform2D());

		// Draw joint
		vs->canvas_item_add_circle(p_canvas_item, gear->joint_position * 100, gear->radius * 10, color);
		vs->canvas_item_add_circle(p_canvas_item, gear->position * 100, gear->radius * 10, color);

		// Set the draw transform
		vs->canvas_item_add_set_transform(p_canvas_item, Transform2D(gear->rotation, gear_pos * 100));

		// Draw the gear radius
		vs->canvas_item_add_circle(p_canvas_item, Vector2(), gear->radius * 100, color);

		// Get gear circumference
		float circ = (float)Math_TAU * gear->radius;

		// Get tooth size
		float tooth_size = (circ / gear->tooth_count) * gear->tooth_size_ratio;

		// Get angle spacing
		float a = (float)Math_TAU / gear->tooth_count;

		// For each tooth
		for (int i = 0; i < gear->tooth_count; i++) {
			// Transform by tooth rotation
			vs->canvas_item_add_set_transform(p_canvas_item, Transform2D(gear->rotation + a * i, gear_pos * 100));

			// Draw tooth
			vs->canvas_item_add_rect(
				p_canvas_item, Rect2(
					gear->radius * 90, -tooth_size * 50,
					gear->radius * 10 + gear->tooth_radius * 100, tooth_size * 100
				), color
			);
		}

		// Set the next gear
		gear = gear->next_gear;
	}
}

void EngineSimulator::_init() {
	engine_starter.instance();
	audio_write = audio_buffer.cursor();
	audio_read = audio_buffer.cursor();
	microphone.set_audio_write(&audio_write);
}

EngineSimulator::EngineSimulator() {
	engine_starter = Ref<EngineStarter>();
	audio_buffer = RingBuffer<float>();
	microphone = VirtualMicrophone();
	starter_factor = 0;
	time = 0;
}

EngineSimulator::~EngineSimulator() {
	audio_buffer.destroy();
}

void EngineSimulator::_register_methods() {
	register_property<EngineSimulator, float>(
		"starter_factor", 
		&EngineSimulator::set_starter_factor,
		&EngineSimulator::get_starter_factor,
		0
	);
	register_property<EngineSimulator, int>(
		"audio_buffer_size", 
		&EngineSimulator::set_audio_buffer_size,
		&EngineSimulator::get_audio_buffer_size,
		16000
	);
	register_property<EngineSimulator, float>(
		"audio_sample_rate", 
		&EngineSimulator::set_audio_sample_rate,
		&EngineSimulator::get_audio_sample_rate,
		44100
	);

	register_method("simulate", &EngineSimulator::simulate);
	register_method("draw_simulation", &EngineSimulator::draw_simulation);
}