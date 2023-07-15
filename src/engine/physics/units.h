#ifndef UNITS_H
#define UNITS_H

namespace units {
	// Force
	constexpr float N = 1;
	constexpr float lbf = N * 4.44822f;

	// Mass
	constexpr float kg = 1;
	constexpr float g = kg / 1000;
	constexpr float lb = 0.45359237f * kg;

	// Distance
	constexpr float m = 1;
	constexpr float cm = m / 100;
	constexpr float mm = m / 1000;
	constexpr float km = m * 1000;

	// Time
    constexpr float sec = 1;
    constexpr float minute = 60 * sec;
    constexpr float hour = 60 * minute;

    // Torque
    constexpr float Nm = N * m;
    constexpr float ft_lb = foot * lbf;

    // Power
    constexpr float W = Nm / sec;
    constexpr float kW = W * 1000;
    constexpr float hp = 745.699872f * W;

    // Volume
    constexpr float m3 = 1;
    constexpr float cc = cm * cm * cm;
    constexpr float mL = cc;
    constexpr float L = mL * 1000;
    constexpr float cubic_feet = foot * foot * foot;
    constexpr float cubic_inches = inch * inch * inch;
    constexpr float gal = 3.785411784f * L;

    // Molecular
    constexpr float mol = 1;
    constexpr float kmol = mol / 1000;
    constexpr float mmol = mol / 1000000;
    constexpr float lbmol = mol * 453.59237f;

    // Flow-rate (moles)
    constexpr float mol_per_sec = mol / sec;
    constexpr float scfm = 0.002641f * lbmol / minute;

    // Area
    constexpr float m2 = 1;
    constexpr float cm2 = cm * cm;

    // Pressure
    constexpr float Pa = 1;
    constexpr float kPa = Pa * 1000;
    constexpr float MPa = Pa * 1000000;
    constexpr float atm = 101.325f * kPa;

    constexpr float mbar = Pa * 100;
    constexpr float bar = mbar * 1000;

    constexpr float psi = lbf / (inch * inch);
    constexpr float psig = psi;
    constexpr float inHg = Pa * 3386.3886666666713f;
    constexpr float inH2O = inHg * 0.0734824f;

    // Temperature
    constexpr float K = 1;
    constexpr float K0 = 273.15f;
    constexpr float C = K;
    constexpr float F = (5.0f / 9) * K;
    constexpr float F0 = -459.67f;

    // Energy
    constexpr float J = 1;
    constexpr float kJ = J * 1000;
    constexpr float MJ = J * 1000000;

    // Angles
    constexpr float rad = 1;
    constexpr float deg = rad * (pi / 180);

	// Other constants
	constexpr float pi = 3.14159265359f;
    constexpr float R = 8.31446261815324f;
    constexpr float root_2 = 1.41421356237f;
    constexpr float e = 2.718281828459045f;
	constexpr float air_mass = (28.97f * units::g) / units::mol;
}

#endif