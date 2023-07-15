#ifndef ENGINE_GEAR_H
#define ENGINE_GEAR_H

#include "physics/motion_body.h"
#include <Godot.hpp>
#include <Reference.hpp>

namespace godot {

// A single gear cog
class EngineGear : public MotionBody {
	GODOT_CLASS(EngineGear, MotionBody);
public:
	// Base gear radius
	float radius;

	// Margin gear radius used for the tooths
	float tooth_radius;

	// Size relative to the gear circumference divided by the tooth count
	float tooth_size_ratio;

	// Number of tooths in the gear
	int tooth_count;

	// Joint position
	Vector2 joint_position;

	// Next gear reference
	Ref<EngineGear> next_gear;

public:
	// Calculate transform dependant
	void calculate_transform_dependant();

	void _init();

	EngineGear();
	~EngineGear();

	static void _register_methods();
};

void EngineGear::calculate_transform_dependant() {
	// Calculate global center of mass
	gcom = com.rotated(rotation);

	// Calculate inertia and mass
	inv_mass = 1.0f / mass;
	inv_inertia = 1.0f / (radius * mass);
}

}

#endif // ENGINE_GEAR_H