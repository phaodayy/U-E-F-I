#pragma once
#include <cmath>

// Use PAOD-style math structures as the main primitives
#include "Common/math.h"

// Redefine SDK math helpers to bridge with UE4 structures
#include "Utils/ue4math/ue4math.h"
#include "Utils/ue4math/vector.h"
#include "Utils/ue4math/quat.h"
#include "Utils/ue4math/matrix.h"
#include "Utils/ue4math/transform.h"

// Use aliases if necessary to avoid conflict, but here we keep them separate as per project style.
// FVector = UE4-struct
// Vector3 = PAOD-struct

namespace UnrealMath {
    inline FMatrix TransformToMatrix(const FTransform& transform) {
        FMatrix matrix;
        matrix._41 = transform.Translation.X;
        matrix._42 = transform.Translation.Y;
        matrix._43 = transform.Translation.Z;

        float x2 = transform.Rotation.X + transform.Rotation.X;
        float y2 = transform.Rotation.Y + transform.Rotation.Y;
        float z2 = transform.Rotation.Z + transform.Rotation.Z;
        float xx2 = transform.Rotation.X * x2, yy2 = transform.Rotation.Y * y2, zz2 = transform.Rotation.Z * z2;

        matrix.M[0][0] = (1.0f - (yy2 + zz2)) * transform.Scale3D.X;
        matrix.M[1][1] = (1.0f - (xx2 + zz2)) * transform.Scale3D.Y;
        matrix.M[2][2] = (1.0f - (xx2 + yy2)) * transform.Scale3D.Z;

        float yz2 = transform.Rotation.Y * z2, wx2 = transform.Rotation.W * x2;
        matrix.M[2][1] = (yz2 - wx2) * transform.Scale3D.Z;
        matrix.M[1][2] = (yz2 + wx2) * transform.Scale3D.Y;

        float xy2 = transform.Rotation.X * y2, wz2 = transform.Rotation.W * z2;
        matrix.M[1][0] = (xy2 - wz2) * transform.Scale3D.Y;
        matrix.M[0][1] = (xy2 + wz2) * transform.Scale3D.X;

        float xz2 = transform.Rotation.X * z2, wy2 = transform.Rotation.W * y2;
        matrix.M[2][0] = (xz2 + wy2) * transform.Scale3D.Z;
        matrix.M[0][2] = (xz2 - wy2) * transform.Scale3D.X;

        matrix._14 = 0.0f; matrix._24 = 0.0f; matrix._34 = 0.0f; matrix._44 = 1.0f;
        return matrix;
    }

    inline FMatrix MatrixMultiplication(const FMatrix& M1, const FMatrix& M2) {
        FMatrix Out;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                Out.M[i][j] = M1.M[i][0] * M2.M[0][j] + M1.M[i][1] * M2.M[1][j] + M1.M[i][2] * M2.M[2][j] + M1.M[i][3] * M2.M[3][j];
            }
        }
        return Out;
    }
}
