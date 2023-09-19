//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_LIGHT_H
#define XD_RT_LIGHT_H

#include <memory>
#include "CoreTypes.h"
#include "Distribution.h"
#include "MathType.h"
namespace xd {
class Light : public std::enable_shared_from_this<Light> {
public:
	virtual ~Light() = default;
	virtual Vector3f getDirection(const Vector3f& point, HitRecord& rec) const = 0;
	virtual ColorRGB getIntensity(const Vector3f& direction) const = 0;
	virtual bool isDeltaPosition() const = 0;
	virtual bool isDeltaDirection() const = 0;
};

class PointLight : public Light {
public:
	PointLight(const Vector3f& position, const ColorRGB& intensity);
	Vector3f getDirection(const Vector3f& point, HitRecord& rec) const override;
	ColorRGB getIntensity(const Vector3f& direction) const override;
	bool isDeltaPosition() const override { return true; }
	bool isDeltaDirection() const override { return false; }

protected:
	Vector3f position;
	ColorRGB intensity;
};
class DomeLight : public Light {
public:
	DomeLight(const std::shared_ptr<SphereTexture3f>& dome);
	Vector3f getDirection(const Vector3f& point, HitRecord& rec) const override;
	ColorRGB getIntensity(const Vector3f& direction) const override;
	bool isDeltaPosition() const override;
	bool isDeltaDirection() const override;

protected:
	std::shared_ptr<SphereTexture3f> dome;
	UniformHemisphere dis{};
};
}  // namespace xd
#endif	// XD_RT_LIGHT_H
