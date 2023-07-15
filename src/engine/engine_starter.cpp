#include "engine_starter.h"
// #include <xmmintrin.h>

using namespace godot;

static inline float fposmod(float x, float y) {
	return Math::fmod(Math::fmod(x, y) + y, y);
}

// static inline float rsqrt(float x) {
// 	const __m128 a = _mm_set_ss(x);
//     __m128 r = _mm_rsqrt_ss(a);
// 	// r = _mm_add_ss(_mm_mul_ss(_mm_set_ss(1.5f), r), _mm_mul_ss(_mm_mul_ss(_mm_mul_ss(a, _mm_set_ss(-0.5f)), r), _mm_mul_ss(r, r)));
// 	return _mm_cvtss_f32(r);
// }

void EngineStarter::step(float p_dt)  {
	// Current updated gear
	Ref<EngineGear> gear = main_gear;

	// First gear isn't valid, abort
	ERR_FAIL_COND(!gear.is_valid());

	// Get spring force and drag delta
	float spring_force_dt = spring_force * p_dt;
	float spring_drag_dt = Math::clamp(
		spring_drag * p_dt, 0.0f, 1.0f
	);
	float torque_drag_dt = Math::clamp(
		4.0f * p_dt, 0.0f, 1.0f
	);

	// Get starter desired angular velocity
	float anv = starter_rpm * (float)Math_TAU / 60;
	float acc = starter_acceleration * (float) Math_TAU / 60;
	acc = acc * p_dt * starter;

	// Accelerate gear
	gear->apply_rotation_impulse(
		Math::clamp(anv - gear->angular_velocity, -acc, acc), 
		MotionBody::IMPULSE_MODE_VELOCITY_CHANGE
	);

	// While has a gear to update
	while (gear.is_valid()) {
		// Get next gear
		Ref<EngineGear> next_gear = gear->next_gear;

		// Move gear by velocity
		gear->position += gear->linear_velocity * p_dt;
		gear->rotation += gear->angular_velocity * p_dt;

		// Apply spring force
		gear->apply_central_impulse(
			(gear->joint_position - gear->position) * spring_force_dt,
			MotionBody::IMPULSE_MODE_VELOCITY_CHANGE
		);
		gear->apply_central_impulse(
			-gear->linear_velocity * spring_drag_dt,
			MotionBody::IMPULSE_MODE_VELOCITY_CHANGE
		);

		// Has next gear
		if (next_gear.is_valid()) {
			// Simulate gears transmission
			gears_transmission(gear, next_gear, p_dt);
		}

		// Apply drag
		if (!next_gear.is_valid()) {
			gear->apply_rotation_impulse(
				-gear->angular_velocity * torque_drag_dt,
				MotionBody::IMPULSE_MODE_VELOCITY_CHANGE
			);
		}

		// Commit applied forces of this gear
		gear->commit_forces();

		// Calculate transform dependant
		gear->calculate_transform_dependant();

		// Set next gear
		gear = next_gear;
	}
}

void EngineStarter::gears_transmission(Ref<EngineGear> &p_g0, Ref<EngineGear> &p_g1, float p_dt) {
	// Get collision distance between the two gears
	float col_dst = p_g0->radius + p_g1->radius + p_g0->tooth_radius + p_g1->tooth_radius;

	// Get both gears positions
	Vector2 pos0 = p_g0->position + p_g0->gcom;
	Vector2 pos1 = p_g1->position + p_g1->gcom;

	// Get offset between gears
	Vector2 off = pos1 - pos0;

	// Get distance between gears
	float dst = off.length_squared();

	// Is colliding
	if (dst > 0 && dst <= col_dst * col_dst) {
		// Get reciprocal of distance square root
		float rdst = 1 / Math::sqrt(dst);

		// Get normal
		Vector2 normal = off * rdst;

		// Get collision position
		Vector2 col_pos = pos1 - normal * p_g1->radius;

		// Get both gears tooth pitch
		float ph0 = (float)Math_TAU / p_g0->tooth_count;
		float ph1 = (float)Math_TAU / p_g1->tooth_count;

		// Get both gears tooth width
		float w0 = ph0 * p_g0->tooth_size_ratio;
		float w1 = ph1 * p_g1->tooth_size_ratio;

		// Get both gears tooth time
		float t0 = p_g0->rotation;
		float t1 = p_g1->rotation;

		// Fract both times
		t0 = fposmod(t0 + ph0 * 0.5f, ph0) - ph0 * 0.5f;
		t1 = fposmod(t1 + ph1 * 0.5f + (float)Math_PI, ph1) - ph1 * 0.5f;

		// Get offset between tooths
		float th_off = t1 - t0;

		// Tooths are colliding
		if (abs(th_off) <= w0 * 0.5f + w1 * 0.5f) {
			// Get collision velocity
			Vector2 vel = p_g0->get_point_velocity(col_pos) - p_g1->get_point_velocity(col_pos);

			// Get friction force
			Vector2 fric = vel - normal * normal.dot(vel);
			fric /= (p_g0->inv_mass + p_g1->inv_mass);

			// Apply friction force
			p_g0->apply_impulse(col_pos, -fric);
			p_g1->apply_impulse(col_pos,  fric);
		}
	}
}

void EngineStarter::record_audio_sample(VirtualMicrophone &p_microphone) {
	// Current updated gear
	Ref<EngineGear> gear = main_gear;

	// First gear isn't valid, abort
	ERR_FAIL_COND(!gear.is_valid());

	// While has a gear to update
	while (gear.is_valid()) {
		// Get gear velocity
		float vel = gear->linear_velocity.y;

		// Get gear position
		float pos = gear->position.x;

		// Record sample
		p_microphone.record_audio_sample(vel * 32, pos);

		// Set next gear
		gear = gear->next_gear;

		// break;
	}
}

void EngineStarter::_init() {
	Ref<EngineGear> g0;
	Ref<EngineGear> g1;
	Ref<EngineGear> g2;

	g0.instance();
	g1.instance();
	g2.instance();

	g0->tooth_count = 32;
	g0->radius = 0.25f;
	g0->tooth_radius = 0.05f;
	g0->tooth_size_ratio = 0.6f;
	g0->joint_position = g0->position = Vector2(0, 0);
	g0->com = Vector2(0, 0);

	g1->tooth_count = 32;
	g1->radius = 0.25f;
	g1->tooth_radius = 0.05f;
	g1->tooth_size_ratio = 0.6f;
	g1->joint_position = g1->position = g0->position + Vector2(0.25f + 0.25f + 0.05f, 0);
	g1->com = Vector2(0, 0);

	g2->tooth_count = 32;
	g2->radius = 0.25f;
	g2->tooth_radius = 0.05f;
	g2->tooth_size_ratio = 0.6f;
	g2->joint_position = g2->position = g1->position + Vector2(0.25f + 0.25f + 0.05f, 0);
	g2->com = Vector2(0, 0);

	g0->next_gear = g1;
	g1->next_gear = g2;

	g0->calculate_transform_dependant();
	g1->calculate_transform_dependant();
	g2->calculate_transform_dependant();

	// Set the main gear
	main_gear = g0;
}

EngineStarter::EngineStarter() {
	main_gear = Ref<EngineGear>();
	starter = 0.0f;
	starter_rpm = 500.0f;
	starter_acceleration = 8000.0f;

	spring_force = 40000.0f;
	spring_drag = 128.0f;
}

EngineStarter::~EngineStarter() {

}

void EngineStarter::_register_methods() {
	
}