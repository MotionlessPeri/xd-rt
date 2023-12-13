//
// Created by Frank on 2023/9/13.
//

#ifndef XD_RT_ENUMS_H
#define XD_RT_ENUMS_H
namespace xd {
enum class HitAccelMethod { NO_ACCEL, BVH, EMBREE };
enum class HitSolverType { NAIVE, BVH, EMBREE };
enum class FrameCategory {
	LOCAL,	// the frame where radiance calculation is taken place. This frame is commonly built
			// with hitting point, geom normal and surface derivatives
	MODEL,	// the frame the Shape class lies in
	WORLD	// the frame of the world
};
}  // namespace xd
#endif	// XD_RT_ENUMS_H
