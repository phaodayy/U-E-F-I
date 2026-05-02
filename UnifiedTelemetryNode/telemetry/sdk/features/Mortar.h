#pragma once
#include <cmath>
#include <numbers>

#define _USE_MATH_DEFINES

namespace Mortar {
    // PUBG Mortar Constants
    constexpr double V0 = 83.0; // Initial velocity in m/s
    constexpr double G = 9.8;   // Gravity in m/s^2

    /**
     * Calculates the required pitch angle for a Mortar to hit a target.
     * @param distance Horizontal distance to target in meters.
     * @param heightDiff Vertical height difference (Target Z - Player Z) in meters.
     * @return The required pitch angle in degrees (high angle), or -1.0 if unreachable.
     */
    inline double GetPitch(double distance, double heightDiff) {
        if (distance < 1.0) return 89.0; // Too close, aim almost straight up

        double v2 = V0 * V0;
        double v4 = v2 * v2;
        double g = G;
        double x = distance;
        double y = heightDiff;

        // Ballistic trajectory formula for angle theta:
        // tan(theta) = (v^2 + sqrt(v^4 - g(gx^2 + 2yv^2))) / (gx)
        // We use '+' for high-angle fire (typical for Mortars)
        
        double rootTerm = v4 - g * (g * x * x + 2.0 * y * v2);
        
        if (rootTerm < 0) {
            // Target is out of range
            return -1.0;
        }

        double tanTheta = (v2 + std::sqrt(rootTerm)) / (g * x);
        double thetaRad = std::atan(tanTheta);
        double thetaDeg = thetaRad * (180.0 / 3.14159265358979323846);

        // PUBG Mortar UI usually shows angles between 45 and 90.
        // We return the pitch directly.
        return thetaDeg;
    }
}