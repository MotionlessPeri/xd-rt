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
	explicit Light(uint32_t numSamples) : numSamples(numSamples) {}
	virtual ~Light() = default;
	virtual Vector3f getDirection(const HitRecord& primRec, HitRecord& shadowRec) const = 0;
	virtual ColorRGB getIntensity(const Ray& ray) const = 0;
	virtual bool isDelta() const = 0;
	virtual float getPdf(const Vector3f& dir) const = 0;
	virtual Vector3f sample(const HitRecord& primRec, HitRecord& shadowRec, float& pdf) = 0;
	uint32_t getNumSamples() const { return numSamples; }

protected:
	uint32_t numSamples;
};

class PointLight : public Light {
public:
	PointLight(const Vector3f& position, const ColorRGB& intensity);
	Vector3f getDirection(const HitRecord& primRec, HitRecord& shadowRec) const override;
	ColorRGB getIntensity(const Ray& ray) const override;
	bool isDelta() const override { return true; }
	float getPdf(const Vector3f& dir) const override;
	Vector3f sample(const HitRecord& primRec, HitRecord& shadowRec, float& pdf) override;

protected:
	Vector3f position;
	ColorRGB intensity;
};
class DomeLight : public Light {
public:
	DomeLight(const std::shared_ptr<SphereTexture3f>& dome);
	Vector3f getDirection(const HitRecord& primRec, HitRecord& shadowRec) const override;
	ColorRGB getIntensity(const Ray& ray) const override;
	bool isDelta() const override;
	float getPdf(const Vector3f& dir) const override;
	Vector3f sample(const HitRecord& primRec, HitRecord& shadowRec, float& pdf) override;

protected:
	std::shared_ptr<SphereTexture3f> dome;
	std::shared_ptr<PieceWise2D> dis;
};
}  // namespace xd
#endif	// XD_RT_LIGHT_H
