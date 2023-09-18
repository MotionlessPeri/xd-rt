//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_LIGHT_H
#define XD_RT_LIGHT_H

#include <memory>
#include "CoreTypes.h"
#include "MathType.h"
namespace xd {
class Light : public std::enable_shared_from_this<Light> {
public:
	virtual ~Light() = default;
	virtual ColorRGB getIntensity(const Vector3f& point) const = 0;
	virtual Ray getShadowRay(const Vector3f& point) const = 0;
	virtual bool isDeltaPosition() const = 0;
	virtual bool isDeltaDirection() const = 0;
};

class PointLight : public Light {
public:
	PointLight(const Vector3f& position, const ColorRGB& intensity);
	ColorRGB getIntensity(const Vector3f& point) const override;
	Ray getShadowRay(const Vector3f& point) const override;
	bool isDeltaPosition() const override { return true; }
	bool isDeltaDirection() const override { return false; }

protected:
	Vector3f position;
	ColorRGB intensity;
};
class DomeLight : public Light {};
}  // namespace xd
#endif	// XD_RT_LIGHT_H
