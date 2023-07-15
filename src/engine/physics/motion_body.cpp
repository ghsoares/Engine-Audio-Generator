#include "motion_body.h"

using namespace godot;

void MotionBody::_init() {

}

MotionBody::MotionBody() {
	position = Vector2();
	rotation = 0.0f;

	com = Vector2();
	gcom = Vector2();

	mass = 10.0f;
	inv_inertia = 1.0f;
	inv_mass = 1.0f;

	linear_velocity = Vector2();
	angular_velocity = 0.0f;

	applied_position = Vector2();
	applied_linear_velocity = Vector2();
	applied_rotation = 0.0f;
	applied_angular_velocity = 0.0f;
}

MotionBody::~MotionBody() {

}

void MotionBody::_register_methods() {
	
}