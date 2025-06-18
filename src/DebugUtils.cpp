#include "DebugUtils.h"

void DebugUtils::PrintEV3d(const Eigen::Vector3d& v) {
    std::cout << '(' << v[0] << ", " << v[1] << ", " << v[2] << ')';
}