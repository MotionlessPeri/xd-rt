//
// Created by Frank on 2023/9/13.
//

#ifndef XD_RT_ENUMS_H
#define XD_RT_ENUMS_H
namespace xd {
enum class HitAccelMethod { NO_ACCEL, BVH, EMBREE };
enum class HitSolverType { NAIVE, BVH, EMBREE };
}  // namespace xd
#endif	// XD_RT_ENUMS_H
