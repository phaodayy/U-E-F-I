#pragma once
#include <cmath>

namespace Mortar {
    // PUBG Mortar Constants
    constexpr float V0 = 83.0f; // Initial velocity in m/s
    constexpr float G = 9.8f;   // Gravity in m/s^2

    // Calculates the exact pitch rotation needed to hit a target.
    // distance3D: Straight line distance from camera/mortar to target in meters
    // heightDiff: target.z - mortar.z in meters (positive if target is higher)
    inline float CalculateRequiredPitch(float horizontalDist, float heightDiff) {
        float x = horizontalDist;
        if (x <= 0.0f) return -1.0f;
        float y = heightDiff;
        
        // Equation of trajectory solved for tan(theta):
        // A * tan^2(theta) - x * tan(theta) + (y + A) = 0
        // where A = (g * x^2) / (2 * V0^2)
        float A = (G * x * x) / (2.0f * V0 * V0);
        
        float discriminant = (x * x) - 4.0f * A * (y + A);
        
        if (discriminant < 0.0f) {
            return -1.0f;
        }
        
        // We want the high arc (mortar trajectory), so we add the square root of the discriminant
        float tanTheta = (x + std::sqrt(discriminant)) / (2.0f * A);
        
        float thetaRad = std::atan(tanTheta);
        float thetaDeg = thetaRad * (180.0f / 3.14159265358979323846f);
        
        // In PUBG, Mortar Pitch rotation goes from 0 (which is 45 degrees) to 40 (which is 85 degrees)
        float targetPitch = thetaDeg - 45.0f;
        
        // Clamp to allowed UI bounds (0 to 40 degrees)
        if (targetPitch < 0.0f) targetPitch = 0.0f;
        if (targetPitch > 40.0f) targetPitch = 40.0f;
        
        return targetPitch;
    }
}