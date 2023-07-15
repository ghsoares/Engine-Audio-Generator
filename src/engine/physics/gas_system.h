#ifndef GAS_SYSTEM_H
#define GAS_SYSTEM_H

#include <cmath>
#include "math.h"

// The main gas system, this represents a single container of gas
class GasSytem {
public:
	/* 
	Structure that describes the mixture of specific 
	components in the gas.
	*/
	struct GasMixture {
		// Amount of fuel in the gas
		float fuel = 0;
		// Amount of inert gas, portion that doesn't contribute
		// for reactions
		float inert = 1;
		// Amount of oxygen in the gas
		float o2 = 0;
	};

	/*
	Structure that describes the current state of the gas in the container,
	it uses ideal gas law to describe the main properties of a gas.
	*/
	struct GasState {
		// The amount of molecules in the gas
		float amount = 0;
		// The total energy of the molecules in the gas
		float kinetic_energy = 0;
		// The volume of molecules in the gas
		float volume = 0;
		// Bulk momentum of the gas
		float momentum_x = 0, momentum_y = 0;

		// The mixture of components in the gas
		GasMixture mixture;
	};

	/*
	Structure that describes the parameters used to simulate gas
	flowing from a system to another.
	*/
	struct GasFlowParams {
		float kinetic_flow;
		float delta;
		float direction_x, direction_y;
		float cross_section_area0, cross_section_area1;
		GasSytem *system0, *system1;
	};

protected:
	// The state of the gas system
	GasState m_state;

	/*
	How many ways can the molecules move freely?
	In a gas system dynamics, the molecules can move 
	in the three main axis (x, y, z) and rotate on only two (x, y or pitch and yaw).
	*/
	int m_degrees_of_freedom = 5;

	// Those I ain't sure, it's flow rate, but of what
	// specifically I don't know
	float m_choked_flow_limit = 0;
	float m_choked_flow_rate = 0;

	// The dimensions of the container
	float m_width = 0;
	float m_height = 0;

	// The gas density in both axis
	float m_density_x = 0;
	float m_density_y = 0;

public:
	// --INITIALIZERS-- //
	// Set the main geometry of the system
	void set_geometry(float p_width, float p_height, float p_density_x, float p_density_y);

	// Initializes the system
	void initialize(float p_pressure, float p_volume, float p_temperature, const GasMixture &p_mixture = {}, int p_degrees_of_freedom = 5);

	// Reset the system
	void reset(float p_pressure, float p_temperature, const GasMixture &p_mixture = {});

	// --FUNCTIONS-- //
	// React with passed mixture
	float react(float p_amount, const GasMixture &p_mixture);

	// Dissipate velocity
	void dissipate_velocity(float p_delta, float p_time_constant);

	// Dissipate excess velocity
	void dissipate_excess_velocity();

	// Update velocity
	void update_velocity(float p_delta, float p_beta = 1);

	// Flow with environment, or dissipate for a open space
	float flow(float p_kinematic_flow, float p_delta, float p_env_pressure, float p_env_temperature, const GasMixture &p_mixture = {});

	// Calculate max flow for pressure equilibrium with another system
	float pressure_equilibrium_max_flow(const GasSytem *p_another) const;

	// Calculate max flow for pressure equilibrium with environment
	float pressure_equilibrium_max_flow(float p_env_pressure, float p_env_temperature) const;

	// --CONSTANT EXPRESSIONS-- //
	// Calculate kinetic energy per molecule amount
	inline static constexpr float kinetic_energy_per_mol(float p_temperature, int p_degrees_of_freedom);

	// Calculate heat capacity ratio based on degrees of freedom
	inline static constexpr float heat_capacity_ratio(int p_degrees_of_freedom);

	// --STATIC FUNCTIONS-- //
	// Get choked flow limit
	inline static float choked_flow_limit(int p_degrees_of_freedom);

	// Get choked flow rate
	inline static float choked_flow_rate(int p_degrees_of_freedom);

	// Get flow constant
	static float flow_constant(float p_flow_rate, float p_pressure, float p_pressure_drop, float p_temperature, float p_heat_capacity_ratio);

	// Get flow rate
	static float flow_rate(
		float p_kinematic_flow,
		float p_pressure0, float p_pressure1,
		float p_temperature0, float p_temperature1,
		float p_heat_capacity_ratio,
		float p_choked_flow_limit, 
		float p_choked_flow_rate
	);

	// Flow between systems
	static float flow(const GasFlowParams &p_params);

	// --MODIFIERS-- //
	// Those are like setters, but can exhange between variables

	void set_volume(float p_volume);
	void set_amount(float p_amount);

	// --CHANGERS-- //
	// Those "adds" the amount passed for the system

	void change_volume(float p_volume_change);
	void change_pressure(float p_pressure_change);
	void change_temperature(float p_temperature_change);

	// Change the temperature of the system with amount provided
	void change_temperature(float p_temperature_change, float p_amount);

	void change_kinetic_energy(float p_energy_change);
	void change_mixture(const GasMixture &p_mixture_change);
	void inject_fuel(float p_amount);

	// Loses or gains molecule amount in the system
	float lose_amount(float p_amount, float p_energy_per_mol);
	float gain_amount(float p_amount, float p_energy_per_mol, const GasMixture &p_mixture);

	// --GETTERS-- //
	inline int degrees_of_freedom() const { return m_degrees_of_freedom; }
	inline float width() const { return m_width; }
	inline float height() const { return m_height; }
	inline float density_x() const { return m_density_x; }
	inline float density_y() const { return m_density_y; }
	inline float amount() const { return m_state.amount; }
	inline float amount(float p_volume) const { return (p_volume / volume()) * amount(); }
	inline float kinetic_energy() const { return m_state.kinetic_energy; }
	inline float kinetic_energy(float p_amount) const { return (kinetic_energy() / amount()) * p_amount; }
	inline float kinetic_energy_per_mol() const { return kinetic_energy(1); }
	inline float mass() const { return units::air_mass * amount(); }
	inline float volume() const { return m_state.volume; }
	inline float volume(float p_amount) const { return p_amount * amount() / volume(); }
	inline float velocity_x() const { return amount() == 0 ? 0 : m_state.momentum_x / mass(); }
	inline float velocity_y() const { return amount() == 0 ? 0 : m_state.momentum_y / mass(); }
	inline float fuel_amount() const { return m_state.mixture.fuel * amount(); }
	inline float inert_amount() const { return m_state.mixture.inert * amount(); }
	inline float o2_amount() const { return m_state.mixture.o2 * amount(); }
	inline GasMixture mixture() const { m_state.mixture; }

	inline float approximate_density() const;
	inline float total_energy() const;
	inline float bulk_kinetic_energy() const;
	inline float heat_capacity() const;
	inline float pressure() const;
	inline float dynamic_presssure(float p_density_x, float p_density_y) const;

	GasSytem();
	~GasSytem();
};

void GasSystem::set_volume(float p_volume) {
	change_volume(p_volume - m_state.volume);
}

void GasSystem::set_amount(float p_amount) {
	m_state.kinetic_energy = kinetic_energy(p_amount);
	m_state.amount = p_amount;
}

void GasSystem::change_volume(float p_volume_change) {
	const float vol = volume();
	const float liters = math::pow(vol + p_volume_change, 1 / 3.0f);
	const float area = liters * liters;
	const float liters_change = -p_volume_change / area;
	const float energy_change = liters_change * pressure() * area;

	m_state.volume += liters_change;
	m_state.kinetic_energy += energy_change;
}

void GasSystem::change_pressure(float p_pressure_change) {
	m_state.kinetic_energy += p_pressure_change * m_degrees_of_freedom * 0.5f;
}

void GasSystem::change_temperature(float p_temperature_change) {
	m_state.kinetic_energy += p_temperature * 0.5f * m_degrees_of_freedom * amount() * units::R;
}

void GasSystem::change_temperature(float p_temperature_change, float p_amount) {
	m_state.kinetic_energy += p_temperature * 0.5f * m_degrees_of_freedom * p_amount * units::R;
}

void GasSystem::change_kinetic_energy(float p_energy_change) {
	m_state.kinetic_energy += p_energy_change;
}

void GasSystem::change_mixture(const GasMixture &p_mixture_change) {
	m_state.mixture = p_mixture_change;
}

void GasSystem::inject_fuel(float p_amount) {
	m_state.mixture.fuel = (fuel_amount() + p_amount) / amount();
}

inline constexpr float GasSystem::kinetic_energy_per_mol(float p_temperature, int p_degrees_of_freedom) {
	return 0.5f * p_temperature * units::R * p_degrees_of_freedom;
}

inline constexpr float GasSystem::heat_capacity_ratio(int p_degrees_of_freedom) {
	return 1 + (2.0f / p_degrees_of_freedom);
}

inline float GasSystem::choked_flow_limit(int p_degrees_of_freedom) {
	const float hcr = heat_capacity_ratio(p_degrees_of_freedom);
	return math::pow((2 / (hcr + 1)), hcr / (hcr - 1));
}

inline float GasSystem::choked_flow_rate(int p_degrees_of_freedom) {
	const float hcr = heat_capacity_ratio(p_degrees_of_freedom);
	return math::sqrt(hcr) * math::pow(2 / (hcr + 1), (hcr + 1) / (2 * (hcr - 1)));
}

float GasSystem::flow_constant(float p_flow_rate, float p_pressure, float p_pressure_drop, float p_temperature, float p_heat_capacity_ratio) {
	const float temperature0 = p_temperature;
    const float temperature1 = p_pressure, pressure_temperature = p_pressure - p_pressure_drop; // temperature1 = upstream pressure

    const float choked_flow_limit =
        math::pow((2 / (p_heat_capacity_ratio + 1)), p_heat_capacity_ratio / (p_heat_capacity_ratio - 1));
    const float p_ratio = pressure_temperature / temperature1;

    float flow_rate = 0;
    if (p_ratio <= choked_flow_limit) {
        // Choked flow
        flow_rate = math::sqrt(p_heat_capacity_ratio);
        flow_rate *= math::pow(2 / (p_heat_capacity_ratio + 1), (p_heat_capacity_ratio + 1) / (2 * (p_heat_capacity_ratio - 1)));
    }
    else {
        flow_rate = (2 * p_heat_capacity_ratio) / (p_heat_capacity_ratio - 1);
        flow_rate *= (1 - math::pow(p_ratio, (p_heat_capacity_ratio - 1) / p_heat_capacity_ratio));
        flow_rate = math::sqrt(flow_rate);
        flow_rate *= math::pow(p_ratio, 1 / p_heat_capacity_ratio);
    }

    flow_rate *= temperature1 / math::sqrt(units::R * temperature0);

    return p_flow_rate / flow_rate;
}

float GasSystem::flow_rate(
	float p_kinematic_flow,
    float p_pressure0,
    float p_pressure1,
    float p_temperature0,
    float p_temperature1,
    float p_heat_capacity_ratio,
    float p_choked_flow_limit,
    float p_choked_flow_rate
) {
	if (p_kinematic_flow == 0) return 0;

    float direction;
    float T_0;
    float p_0, p_T; // p_0 = upstream pressure
    if (p_pressure0 > p_pressure1) {
        direction = 1.0;
        T_0 = p_temperature0;
        p_0 = p_pressure0;
        p_T = p_pressure1;
    }
    else {
        direction = -1.0;
        T_0 = p_temperature1;
        p_0 = p_pressure1;
        p_T = p_pressure0;
    }

    const float p_ratio = p_T / p_0;
    float flowRate = 0;
    if (p_ratio <= p_choked_flow_limit) {
        // Choked flow
        flowRate = p_choked_flow_rate;
        flowRate /= math::sqrt(units::R * T_0);
    }
    else {
        const float s = math::pow(p_ratio, 1 / p_heat_capacity_ratio);

        flowRate = (2 * p_heat_capacity_ratio) / (p_heat_capacity_ratio - 1);
        flowRate *= s * (s - p_ratio);
        flowRate = math::sqrt(math::max(flowRate, 0.0) / (units::R * T_0));
    }

    flowRate *= direction * p_0;

    return flowRate * p_kinematic_flow;
}

inline float GasSystem::approximate_density() const {
	return (units::air_mass * amount()) / volume();
}

inline float GasSystem::total_energy() const {
	if (amount() == 0) return 0;

	const float inv_mass = 1 / mass();
	const float vel_x = m_state.momentum_x * inv_mass;
	const float vel_y = m_state.momentum_y * inv_mass;
	const float vel_squared = vel_x * vel_x + vel_y * vel_y;

	return kinetic_energy() + 0.5f * mass() * vel_squared;
}

inline float GasSystem::bulk_kinetic_energy() const {
	const float m = mass();
	if (m == 0) return 0;

	const float inv_mass = 1 / m;
	const float vel_x = m_state.momentum_x * inv_mass;
	const float vel_y = m_state.momentum_y * inv_mass;
	const float vel_squared = vel_x * vel_x + vel_y * vel_y;

	return 0.5f * m * vel_squared;
}

inline float GasSystem::heat_capacity() const {
	if (amount() == 0 || kinetic_energy() == 0) return 0;

	const float hcr = heat_capacity_ratio();
	const float static_pressure = pressure();
	const float density = approximate_density();
	
	return math::sqrt(static_pressure * hcr / density);
}

inline float GasSystem::pressure() const {
	const float vol = volume();
	return (vel != 0) ? (kinetic_energy() / (0.5f * m_degrees_of_freedom * vol)) : 0;
}

inline float GasSystem::dynamic_presssure(float p_density_x, float p_density_y) const {
	if (amount() == 0 || kinetic_energy() == 0) return 0;

	const float inv_mass = 1 / m;
	const float vel = inv_mass * (p_density_x * m_state.momentum_x + p_density_y * m_state.momentum_y);

	if (vel <= 0) return 0;

	const float hcr = heat_capacity_ratio();
	const float static_pressure = pressure();
	const float density = approximate_density();
	const float heat_squared = static_pressure * hcr / density;
	const float mach_number_squared = vel * vel / heat_squared;

	const float x = 1 + ((hcr - 1) / 2) * mach_number_squared;
	float xd;
	switch (m_degrees_of_freedom) {
		case 3: {
			x_d = x * x * x * x * x;
		} break;
		case 5: {
			const float x2 = x * x;
			const float x3 = x2 * x;
			xd = x3 * x3 * x;
		} break;
		default: xd = x;
	}

	return static_pressure * (math::sqrt(xd) - 1);
}

#endif // GAS_SYSTEM_H