//
// Created by Frank on 2024/1/6.
//

#ifndef XD_RT_CONTEXTS_H
#define XD_RT_CONTEXTS_H
#include "MathTypes.h"
namespace xd {
// heavily inspired by pbrt-v4 and mitsuba
struct TextureEvalContext {
	TextureEvalContext() = default;
	TextureEvalContext(const TextureEvalContext& other) = default;
	TextureEvalContext(TextureEvalContext&& other) noexcept = default;
	TextureEvalContext& operator=(const TextureEvalContext& other) = default;
	TextureEvalContext& operator=(TextureEvalContext&& other) noexcept = default;
	explicit TextureEvalContext(Vector3f dir) : p(std::move(dir)) {}

	TextureEvalContext(Vector3f p,
					   Vector2f uv,
					   Vector3f dpdx,
					   Vector3f dpdy,
					   Vector3f dudx,
					   Vector3f dudy,
					   Vector3f dvdx,
					   Vector3f dvdy,
					   Vector3f n)
		: p(std::move(p)),
		  uv(std::move(uv)),
		  dpdx(std::move(dpdx)),
		  dpdy(std::move(dpdy)),
		  dudx(std::move(dudx)),
		  dudy(std::move(dudy)),
		  dvdx(std::move(dvdx)),
		  dvdy(std::move(dvdy)),
		  n(std::move(n))
	{
	}

	Vector3f p;
	Vector2f uv;
	Vector3f dpdx, dpdy;
	Vector3f dudx, dudy, dvdx, dvdy;
	Vector3f n;
};
}  // namespace xd
#endif	// XD_RT_CONTEXTS_H
