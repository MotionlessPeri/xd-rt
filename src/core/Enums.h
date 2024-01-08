//
// Created by Frank on 2023/9/13.
//

#ifndef XD_RT_ENUMS_H
#define XD_RT_ENUMS_H
namespace xd {
enum class HitAccelMethod { UNKNOWN, NO_ACCEL, BVH, EMBREE };
enum class HitSolverType { UNKNOWN, NAIVE, BVH, EMBREE };
enum class FrameCategory {
	UNKNOWN,
	LOCAL,	// the frame where radiance calculation is taken place. This frame is commonly built
			// with hitting point, geom normal and surface derivatives
	MODEL,	// the frame the Shape class lies in
	WORLD	// the frame of the world
};
enum class MaterialType { PHYSICAL };
enum class ColorSpace { UNKNOWN, LINEAR, SRGB };
enum class PixelFormat {
	FORMAT_UNKNOWN,
	FORMAT_R8G8B8_UNORM,
	FORMAT_R8G8B8_SRGB,
	FORMAT_R8G8B8A8_UNORM,
	FORMAT_R8G8B8A8_SRGB,
	FORMAT_R32G32B32_SFLOAT,
	FORMAT_R32G32B32A32_SFLOAT,
};
enum class PixelComponentFormat { UNKONWN, U8, SFLOAT };
}  // namespace xd
#endif	// XD_RT_ENUMS_H
