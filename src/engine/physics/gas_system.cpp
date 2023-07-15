#include "gas_system.h"

void GasSystem::set_geometry(float p_width, float p_height, float p_density_x, float p_density_y) {
	m_width = p_width;
	m_height = p_height;
	m_density_x = p_density_x;
	m_density_y = p_density_y;
}

void GasSystem::initialize(float p_pressure, float p_volume, float p_temperature, const GasMixture &p_mixture, int p_degrees_of_freedom) {
	m_degrees_of_freedom = p_degrees_of_freedom;
	m_state.amount = p_pressure * p_volume / (units::R * p_temperature);
	m_state.volume = p_volume;
	m_state.kinetic_energy = p_temperature * (0.5f * p_degrees_of_freedom * m_state.amount * units::R);
	m_state.mixture = p_mixture;
	m_state.momentum_x = m_state.momentum_y = 0;

	const float hcr = heat_capacity_ratio();
	m_choked_flow_limit = choked_flow_limit(p_degrees_of_freedom);
	m_choked_flow_rate = choked_flow_rate(p_degrees_of_freedom);
}

void GasSystem::reset(float p_pressure, float p_temperature, const GasMixture &p_mixture = {})  {
	m_state.amount = p_pressure * volume() / (units::R * p_temperature);
	m_state.kinetic_energy = p_temperature * (0.5f * p_degrees_of_freedom * m_state.amount * units::R);
	m_state.mixture = p_mixture;
	m_state.momentum_x = m_state.momentum_y = 0;
}

void GasSystem::react(float p_amount, const GasMixture &p_mixture) {
	const float mix_fuel = p_mixture.fuel * p_amount;
	const float mix_o2 = p_mixture.o2 * p_amount;

	const float system_fuel = fuel_amount();
	const float system_o2 = o2_amount();
	const float system_inert = inert_amount();
	const float system_amount = amount();

	constexpr float o2_ratio = 25.0f / 2;
	constexpr float fuel_ratio = 2.0f / 25;
	constexpr float output_input_ratio = (16.0f + 18.0f) / (25 + 2);

	const float ideal_fuel = fuel_ratio * mix_o2;
	const float ideal_o2 = o2_ratio * mix_fuel;

	const float min_fuel = math::min(math::min(system_fuel, mix_fuel), ideal_fuel);
	const float min_o2 = math::min(math::min(system_o2, mix_o2), ideal_o2);

	const float reactants = min_fuel + min_o2;
	const float products = output_input_ratio * reactants;
	const float amount_change = products - reactants;

	m_state.amount += amount_change;

	const float new_fuel = system_fuel - min_fuel;
	const float new_o2 = system_o2 - min_o2;
	const float new_inert = system_inert + products;
	const float new_amount = system_amount + amount_change;

	if (new_amount != 0) {
		m_state.mixture.fuel = new_fuel / new_amount;
		m_state.mixture.inert = new_inert / new_amount;
		m_state.mixture.o2 = new_o2 / new_amount;
	} else {
		m_state.mixture.fuel = m_state.mixture.inert = m_state.mixture.o2 = 0;
	}

	return min_fuel;
}

void GasSystem::dissipate_velocity(float p_delta, float p_time_constant) {
	if (amount() == 0) return;

    const float inv_mass = 1 / mass();
    const float vel_x = m_state.momentum_x * inv_mass;
    const float vel_y = m_state.momentum_y * inv_mass;
    const float vel_squared =
        vel_x * vel_x + vel_y * vel_y;

    const float s = p_delta / (p_delta + p_time_constant);
    m_state.momentum_x = m_state.momentum_x * (1 - s);
    m_state.momentum_y = m_state.momentum_y * (1 - s);

    const float new_velocity_x = m_state.momentum_x * inv_mass;
    const float new_velocity_y = m_state.momentum_y * inv_mass;
    const float newVelocity_squared =
        new_velocity_x * new_velocity_x + new_velocity_y * new_velocity_y;

    const float energy_change = 0.5f * mass() * (vel_squared - newVelocity_squared);
    m_state.kinetic_energy += energy_change;
}

void GasSystem::dissipate_excess_velocity() {
	const float vel_x = velocity_x();
	const float vel_y = velocity_y();
	const float vel_squared = vel_x * vel_x + vel_y * vel_y;
	const float heat_cap = heat_capacity();
	const float heat_cap_squared = heat_cap * heat_cap;

	if (heat_cap_squared >= vel_squared || vel_squared == 0) return;

	const float k_squared = heat_cap_squared / vel_squared;
	const float energy = math::sqrt(k_squared);

	m_state.momentum_x *= energy;
	m_state.momentum_y *= energy;

	m_state.kinetic_energy += 0.5f * mass() * (vel_squared - heat_cap_squared);

	if (m_state.kinetic_energy < 0) m_state.kinetic_energy = 0;
}

void GasSystem::update_velocity(float p_delta, float p_beta) {
	if (amount() == 0) return;

    const float depth = volume() / (m_width * m_height);
    
    float d_momentum_x = 0;
    float d_momentum_y = 0;

    const float p0 = dynamic_presssure(m_density_x, m_density_y);
    const float p1 = dynamic_presssure(-m_density_x, -m_density_y);
    const float p2 = dynamic_presssure(m_density_y, m_density_x);
    const float p3 = dynamic_presssure(-m_density_y, -m_density_x);

    const float p_sa_0 = p0 * (m_height * depth);
    const float p_sa_1 = p1 * (m_height * depth);
    const float p_sa_2 = p2 * (m_width * depth);
    const float p_sa_3 = p3 * (m_width * depth);

    d_momentum_x += p_sa_0 * m_density_x;
    d_momentum_y += p_sa_0 * m_density_y;

    d_momentum_x -= p_sa_1 * m_density_x;
    d_momentum_y -= p_sa_1 * m_density_y;

    d_momentum_x += p_sa_2 * m_density_y;
    d_momentum_y += p_sa_2 * m_density_x;

    d_momentum_x -= p_sa_3 * m_density_y;
    d_momentum_y -= p_sa_3 * m_density_x;

    const float m = mass();
    const float inv_m = 1 / m;
    const float v0_x = m_state.momentum_x * inv_m;
    const float v0_y = m_state.momentum_y * inv_m;

    m_state.momentum_x -= d_momentum_x * p_delta * p_beta;
    m_state.momentum_y -= d_momentum_y * p_delta * p_beta;

    const float v1_x = m_state.momentum_x * inv_m;
    const float v1_y = m_state.momentum_y * inv_m;

    m_state.kinetic_energy -= 0.5f * m * (v1_x * v1_x - v0_x * v0_x);
    m_state.kinetic_energy -= 0.5f * m * (v1_y * v1_y - v0_y * v0_y);

    if (m_state.kinetic_energy < 0) m_state.kinetic_energy = 0;
}

float GasSystem::flow(float p_kinematic_flow, float p_delta, float p_env_pressure, float p_env_temperature, const GasMixture &p_mixture = {}) {
	const float max_flow = pressure_equilibrium_max_flow(p_env_pressure, p_env_temperature);
    float flow = p_delta * flow_rate(
        kinetic_flow,
        pressure(),
        p_env_pressure,
        temperature(),
        p_env_temperature,
        heat_capacity_ratio(),
        m_choked_flow_limit,
        m_choked_flow_rate);

    if (math::abs(flow) > math::abs(max_flow)) {
        flow = max_flow;
    }

    if (flow < 0) {
        const float bulk_energy_0 = bulk_kinetic_energy();
        gain_amount(-flow, kinetic_energy_per_mol(p_env_temperature, m_degrees_of_freedom), p_mixture);
        const float bulk_energy_1 = bulk_kinetic_energy();

        m_state.kinetic_energy += (bulk_energy_1 - bulk_energy_0);
    } else {
        const float starting_n = amount();
        lose_amount(flow, kinetic_energy_per_mol());

        m_state.momentum_x -= (flow / starting_n) * m_state.momentum_x;
        m_state.momentum_y -= (flow / starting_n) * m_state.momentum_y;
    }

    return flow;
}

float GasSystem::flow(const GasFlowParams &p_params) {
	GasSystem *source = nullptr, *sink = nullptr;
    float source_pressure = 0, sink_pressure = 0;
    float dx, dy;
    float source_cross_section = 0, sink_cross_section = 0;
    float direction = 0;

    const float P_0 =
        p_params.system0->pressure()
        + p_params.system0->dynamic_presssure(p_params.direction_x, p_params.direction_y);
    const float P_1 =
        p_params.system1->pressure()
        + p_params.system1->dynamic_presssure(-p_params.direction_x, -p_params.direction_y);

    if (P_0 > P_1) {
        dx = p_params.direction_x;
        dy = p_params.direction_y;
        source = p_params.system0;
        sink = p_params.system1;
        source_pressure = P_0;
        sink_pressure = P_1;
        source_cross_section = p_params.cross_section_area0;
        sink_cross_section = p_params.cross_section_area1;
        direction = 1;
    }
    else {
        dx = -p_params.direction_x;
        dy = -p_params.direction_y;
        source = p_params.system1;
        sink = p_params.system0;
        source_pressure = P_1;
        sink_pressure = P_0;
        source_cross_section = p_params.cross_section_area1;
        sink_cross_section = p_params.cross_section_area0;
        direction = -1;
    }

    float flow = p_params.delta * flow_rate(
        p_params.kinetic_flow,
        source_pressure,
        sink_pressure,
        source->temperature(),
        sink->temperature(),
        source->heat_capacity_ratio(),
        source->m_choked_flow_limit,
        source->m_choked_flow_rate);

    const float max_flow = source->pressure_equilibrium_max_flow(sink);
    flow = math::clamp(flow, 0, 0.9f * source->amount());

    const float fraction = flow / source->amount();
    const float fraction_volume = fraction * source->volume();
    const float fraction_mass = fraction * source->mass();
    const float remaining_mass = (1 - fraction) * source->mass();

    if (flow != 0) {
        // - Stage 1
        // Fraction flows from source to sink.

        const float bulk_energy_src0 = source->bulk_kinetic_energy();
        const float bulk_energy_sink0 = sink->bulk_kinetic_energy();

        const float s0 = source->total_energy() + sink->total_energy();

        const float energy_per_mol = source->kinetic_energy_per_mol();
        sink->gain_amount(flow, energy_per_mol, source->mixture());
        source->lose_amount(flow, energy_per_mol);

        const float s1 = source->total_energy() + sink->total_energy();

        const float dp_x = source->m_state.momentum_x * fraction;
        const float dp_y = source->m_state.momentum_y * fraction;
        source->m_state.momentum_x -= dp_x;
        source->m_state.momentum_y -= dp_y;

        sink->m_state.momentum_x += dp_x;
        sink->m_state.momentum_y += dp_y;

        const float bulk_energy_src1 = source->bulk_kinetic_energy();
        const float bulk_energy_sink1 = sink->bulk_kinetic_energy();

        sink->m_state.kinetic_energy -= ((bulk_energy_src1 + bulk_energy_sink1) - (bulk_energy_src0 + bulk_energy_sink0));
    }
    
    const float source_mass = source->mass();
    const float inv_source_mass = 1 / source_mass;
    const float sink_mass = sink->mass();
    const float inv_sink_mass = 1 / sink_mass;

    const float heat_cap_source = source->heat_capacity();
    const float heat_cap_sink = sink->heat_capacity();

    const float source_initial_momentum_x = source->m_state.momentum_x;
    const float source_initial_momentum_y = source->m_state.momentum_y;

    const float sink_initial_momentum_x = sink->m_state.momentum_x;
    const float sink_initial_momentum_y = sink->m_state.momentum_y;

    // Momentum in fraction

    if (sink_cross_section != 0) {
        const float sink_fraction_velocity =
            math::clamp((fraction_volume / sink_cross_section) / p_params.delta, 0, heat_cap_sink);
        const float sink_fraction_velocity_squared = sink_fraction_velocity * sink_fraction_velocity;
        const float sink_fraction_velocity_x = sink_fraction_velocity * dx;
        const float sink_fraction_velocity_y = sink_fraction_velocity * dy;
        const float sink_fraction_momentum_x = sink_fraction_velocity_x * fraction_mass;
        const float sinkFractionMomentum_y = sink_fraction_velocity_y * fraction_mass;

        sink->m_state.momentum_x += sink_fraction_momentum_x;
        sink->m_state.momentum_y += sinkFractionMomentum_y;
    }

    if (source_cross_section != 0 && source_mass != 0) {
        const float source_fraction_velocity =
            math::clamp((fraction_volume / source_cross_section) / p_params.delta, 0, heat_cap_source);
        const float source_fraction_velocity_squared = source_fraction_velocity * source_fraction_velocity;
        const float source_fraction_velocity_x = source_fraction_velocity * dx;
        const float source_fraction_velocity_y = source_fraction_velocity * dy;
        const float source_fraction_momentum_x = source_fraction_velocity_x * fraction_mass;
        const float source_fraction_momentum_y = source_fraction_velocity_y * fraction_mass;

        source->m_state.momentum_x += source_fraction_momentum_x;
        source->m_state.momentum_y += source_fraction_momentum_y;
    }

    if (source_mass != 0) {
        // Energy conservation
        const float source_velocity0_x = source_initial_momentum_x * inv_source_mass;
        const float source_velocity0_y = source_initial_momentum_y * inv_source_mass;

        const float source_velocity1_x = source->m_state.momentum_x * inv_source_mass;
        const float source_velocity1_y = source->m_state.momentum_y * inv_source_mass;

        source->m_state.kinetic_energy -=
            0.5f * source_mass
            * (source_velocity1_x * source_velocity1_x - source_velocity0_x * source_velocity0_x);

        source->m_state.kinetic_energy -=
            0.5f * source_mass
            * (source_velocity1_y * source_velocity1_y - source_velocity0_y * source_velocity0_y);
    }

    if (sink_mass > 0) {
        const float sink_velocity0_x = sink_initial_momentum_x * inv_sink_mass;
        const float sink_velocity0_y = sink_initial_momentum_y * inv_sink_mass;

        const float sink_velocity1_x = sink->m_state.momentum_x * inv_sink_mass;
        const float sink_velocity1_y = sink->m_state.momentum_y * inv_sink_mass;

        sink->m_state.kinetic_energy -=
            0.5f * sink_mass
            * (sink_velocity1_x * sink_velocity1_x - sink_velocity0_x * sink_velocity0_x);

        sink->m_state.kinetic_energy -=
            0.5f * sink_mass
            * (sink_velocity1_y * sink_velocity1_y - sink_velocity0_y * sink_velocity0_y);
    }

    if (sink->m_state.kinetic_energy < 0) {
        sink->m_state.kinetic_energy = 0;
    }

    if (source->m_state.kinetic_energy < 0) {
        source->m_state.kinetic_energy = 0;
    }

    return flow * direction;
}

float GasSystem::lose_amount(float p_amount, float p_energy_per_mol) {
	m_state.kinetic_energy -= p_energy_per_mol * p_amount;
	m_state.amount -= p_amount;
	
	if (m_state.amount < 0) m_state.amount = 0;

	return p_amount;
}

float GasSystem::gain_amount(float p_amount, float p_energy_per_mol, const GasMixture &p_mixture) {
	const float current_amount = m_state.amount;
	const float next_amount = current_amount + p_amount;

	m_state.kinetic_energy += p_amount * p_energy_per_mol;
	m_state.amount = next_amount;

	if (next_amount != 0) {
		m_state.mixture.fuel = (m_state.mixture.fuel * current_amount + p_amount * m_state.mixture.fuel) / next_amount;
		m_state.mixture.inert = (m_state.mixture.inert * current_amount + p_amount * m_state.mixture.inert) / next_amount;
		m_state.mixture.o2 = (m_state.mixture.o2 * current_amount + p_amount * m_state.mixture.o2) / next_amount;
	} else {
		m_state.mixture.fuel = m_state.mixture.inert = m_state.mixture.o2 = 0;
	}

	return -p_amount;
}

float GasSystem::pressure_equilibrium_max_flow(const GasSytem *p_another) const {
    if (pressure() > p_another->pressure()) {
        const float max_flow = (
            (p_another->volume() * kinetic_energy() - volume() * p_another->kinetic()) /
            (p_another->volume() * kinetic_energy_per_mol() + volume() * kinetic_energy_per_mol())
        );
        return math::max(0, math::min(max_flow, amount()));
    } else {
        const float max_flow = (
            (p_another->volume() * kinetic_energy() - volume() * p_another->kinetic()) /
            (p_another->volume() * p_another->kinetic_energy_per_mol() + volume() * p_another->kinetic_energy_per_mol())
        );
        return math::max(0, math::min(max_flow, -p_another->amount()));
    }
}

float GasSystem::pressure_equilibrium_max_flow(float p_env_pressure, float p_env_temperature) const {
    if (pressure() > p_env_pressure) {
        return -(p_env_pressure * (0.5f * m_degrees_of_freedom * volume()) - kinetic_energy()) / kinetic_energy_per_mol();
    }
    else {
        const float energy_per_mol_env = 0.5f * p_env_temperature * units::R * m_degrees_of_freedom;
        return -(p_env_pressure * (0.5f * m_degrees_of_freedom * volume()) - kinetic_energy()) / energy_per_mol_env;
    }
}
