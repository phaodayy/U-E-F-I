#pragma once
#include <cmath>
#include <map>
#include <limits>
#include <utility>

#define _USE_MATH_DEFINES

namespace Mortar {
    // PUBG Mortar Constants
    constexpr double V0 = 83.0; // Initial velocity in m/s
    constexpr double G = 9.8;   // Gravity in m/s^2

    inline double calculateAngle(double distance, double height) {
        double radians = std::atan2(height, distance);
        double degrees = radians * (180.0 / 3.14159265358979323846);
        return degrees;
    }

    inline double calculateR(double L, double betaDeg, double v0 = 83.0, double g = 9.8) {
        double betaRad = betaDeg * (3.14159265358979323846 / 180.0);
        double tanBeta = std::tan(betaRad);

        double v0SquaredOverG = (v0 * v0) / g;
        double v0Fourth = std::pow(v0, 4);
        double gSquared = g * g;

        double sqrtExpression = (v0Fourth / gSquared)
            - ((2 * L * v0 * v0 * tanBeta) / g)
            - (L * L);

        if (sqrtExpression < 0) {
            return -1.0;
        }

        double sqrtResult = std::sqrt(sqrtExpression);
        double numerator = L + tanBeta * (v0SquaredOverG - sqrtResult);
        double denominator = 1.0 + tanBeta * tanBeta;

        return numerator / denominator;
    }

    inline double getDistance(double d, double h) {
        double r = calculateAngle(d, h);
        double m = calculateR(d, r);
        return m >= 0 ? m : 0.0;
    }

    inline std::pair<float, float> FindClosestPitch(float targetDistance) {
        static const std::map<float, float> pitchMap = {
            {0.0f, 700.0f}, {0.5f, 699.0f}, {1.0f, 699.0f}, {1.5f, 699.0f}, {2.0f, 698.0f}, {2.5f, 697.0f}, {3.0f, 696.0f}, {3.5f, 695.0f},
            {4.0f, 693.0f}, {4.5f, 691.0f}, {5.0f, 689.0f}, {5.5f, 687.0f}, {6.0f, 685.0f},
            {6.5f, 682.0f}, {7.0f, 679.0f}, {7.5f, 676.0f}, {8.0f, 673.0f}, {8.5f, 669.0f},
            {9.0f, 666.0f}, {9.5f, 662.0f}, {10.0f, 658.0f}, {10.5f, 653.0f}, {11.0f, 649.0f},
            {11.5f, 644.0f}, {12.0f, 639.0f}, {12.5f, 634.0f}, {13.0f, 629.0f}, {13.5f, 624.0f},
            {14.0f, 618.0f}, {14.5f, 612.0f}, {15.0f, 606.0f}, {15.5f, 600.0f}, {16.0f, 593.0f},
            {16.5f, 587.0f}, {17.0f, 580.0f}, {17.5f, 573.0f}, {18.0f, 566.0f}, {18.5f, 559.0f},
            {19.0f, 551.0f}, {19.5f, 544.0f}, {20.0f, 536.0f}, {20.5f, 528.0f}, {21.0f, 520.0f},
            {21.5f, 512.0f}, {22.0f, 503.0f}, {22.5f, 495.0f}, {23.0f, 486.0f}, {23.5f, 477.0f},
            {24.0f, 468.0f}, {24.5f, 459.0f}, {25.0f, 450.0f}, {25.5f, 440.0f}, {26.0f, 431.0f},
            {26.5f, 421.0f}, {27.0f, 411.0f}, {27.5f, 401.0f}, {28.0f, 391.0f}, {28.5f, 381.0f},
            {29.0f, 371.0f}, {29.5f, 360.0f}, {30.0f, 350.0f}, {30.5f, 339.0f}, {31.0f, 328.0f},
            {31.5f, 317.0f}, {32.0f, 307.0f}, {32.5f, 295.0f}, {33.0f, 284.0f}, {33.5f, 273.0f},
            {34.0f, 262.0f}, {34.5f, 250.0f}, {35.0f, 239.0f}, {35.5f, 228.0f}, {36.0f, 216.0f},
            {36.5f, 204.0f}, {37.0f, 193.0f}, {37.5f, 181.0f}, {38.0f, 169.0f}, {38.5f, 157.0f},
            {39.0f, 145.0f}, {39.5f, 133.0f}, {40.0f, 121.0f}
        };

        float closestPitch = 0.0f;
        float closestDistance = 0.0f;
        float minDiff = (std::numeric_limits<float>::max)();

        for (const auto& pair : pitchMap) {
            float currentDiff = std::abs(pair.second - targetDistance);
            if (currentDiff < minDiff) {
                minDiff = currentDiff;
                closestPitch = pair.first;
                closestDistance = pair.second;
            }
        }

        return { closestPitch, closestDistance };
    }
}