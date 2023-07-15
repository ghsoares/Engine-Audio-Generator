#ifndef MOTION_BODY_H
#define MOTION_BODY_H

#include <Godot.hpp>
#include <Reference.hpp>

namespace godot {

// A single motion body
class MotionBody : public Reference {
	GODOT_CLASS(MotionBody, Reference);
public:
	// Impulse mode
	enum ImpulseMode {
		IMPULSE_MODE_VELOCITY_IMPULSE,
		IMPULSE_MODE_VELOCITY_CHANGE,
		IMPULSE_MODE_POSITION_IMPULSE,
		IMPULSE_MODE_POSITION_CHANGE,
	};

	// Current body transform
	Vector2 position;
	float rotation;

	// Mass of the body
	float mass;

	// Inverse inertia and mass of the body
	float inv_inertia;
	float inv_mass;

	// Center of mass
	Vector2 com;
	Vector2 gcom;

	// Current body velocity
	Vector2 linear_velocity;
	float angular_velocity;

	// Applied forces
	Vector2 applied_position;
	Vector2 applied_linear_velocity;
	float applied_rotation;
	float applied_angular_velocity;

public:
	// Apply central impulse
	void apply_central_impulse(Vector2 p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE);

	// Apply rotation impulse
	void apply_rotation_impulse(float p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE);

	// Apply impulse at position
	void apply_impulse(Vector2 p_pos, Vector2 p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE);

	// Get velocity at centered position
	Vector2 get_point_velocity(Vector2 p_pos);

	// Commit applied forces
	void commit_forces();

	// Calculate transform dependant
	void calculate_transform_dependant();

	void _init();

	MotionBody();
	~MotionBody();

	static void _register_methods();
};

void MotionBody::apply_central_impulse(Vector2 p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE) {
	switch (p_impulse_mode) {
		case IMPULSE_MODE_VELOCITY_IMPULSE: {
			applied_linear_velocity += p_j * inv_mass;
		} break;
		case IMPULSE_MODE_VELOCITY_CHANGE: {
			applied_linear_velocity += p_j;
		} break;
		case IMPULSE_MODE_POSITION_IMPULSE: {
			applied_position += p_j * inv_mass;
		} break;
		case IMPULSE_MODE_POSITION_CHANGE: {
			applied_position += p_j;
		} break;
	}
}

void MotionBody::apply_rotation_impulse(float p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE) {
	switch (p_impulse_mode) {
		case IMPULSE_MODE_VELOCITY_IMPULSE: {
			applied_angular_velocity += p_j * inv_inertia;
		} break;
		case IMPULSE_MODE_VELOCITY_CHANGE: {
			applied_angular_velocity += p_j;
		} break;
		case IMPULSE_MODE_POSITION_IMPULSE: {
			applied_rotation += p_j * inv_inertia;
		} break;
		case IMPULSE_MODE_POSITION_CHANGE: {
			applied_rotation += p_j;
		} break;
	}
}

void MotionBody::apply_impulse(Vector2 p_pos, Vector2 p_j, ImpulseMode p_impulse_mode = IMPULSE_MODE_VELOCITY_IMPULSE) {
	p_pos -= position;
	p_pos -= gcom;
	
	apply_central_impulse(p_j, p_impulse_mode);
	apply_rotation_impulse(p_pos.cross(p_j), p_impulse_mode);
}

Vector2 MotionBody::get_point_velocity(Vector2 p_pos) {
	p_pos -= position;
	p_pos -= gcom;
	
	return Vector2(
		linear_velocity.x - angular_velocity * p_pos.y,
		linear_velocity.y + angular_velocity * p_pos.x
	);
}

void MoionBody::commit_forces() {
	position += applied_position;
	rotation += applied_rotation;
	linear_velocity += applied_linear_velocity;
	angular_velocity += applied_angular_velocity;

	// Reset forces
	applied_position = Vector2();
	applied_rotation = 0.0;
	applied_linear_velocity = Vector2();
	applied_angular_velocity = 0.0;
}

void MotionBody::calculate_transform_dependant() {
	// Calculate global center of mass
	gcom = com.rotated(rotation);

	// Calculate inertia and mass
	inv_mass = 1.0f / mass;
	inv_inertia = 1.0f / mass;
}

}

#endif // MOTION_BODY_H