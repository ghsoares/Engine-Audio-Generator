#include "engine_gear.h"

using namespace godot;

void EngineGear::_init() {

}

EngineGear::EngineGear() {
	radius = 0.5f;
	tooth_radius = 0.1f;
	tooth_size_ratio = 0.5f;
	tooth_count = 8;

	joint_position = Vector2();

	next_gear = Ref<EngineGear>();
}

EngineGear::~EngineGear() {

}

void EngineGear::_register_methods() {
	
}