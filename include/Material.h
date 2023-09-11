//
// Created by Frank on 2023/8/26.
//

#ifndef XD_RT_MATERIAL_H
#define XD_RT_MATERIAL_H
#include <memory>
#include "CoreTypes.h"
#include "MathType.h"
namespace xd {
/**
 * Material is an aggregate of brdfs
 */
class Material {
public:
	virtual ~Material() = default;
	/**
	 * get brdf value
	 * @param hitRecord the geometric properties around hit point
	 * @param wi the minus incident ray direction in world coords
	 * @param wo the out ray direction in world coords
	 * @return
	 */
	virtual Vector3f getBRDF(const HitRecord& hitRecord,
							 const Vector3f& wi,
							 const Vector3f& wo) const = 0;
	virtual Vector3f getDirection(const HitRecord& hitRecord, const Vector3f& wo) const = 0;
};

class MatteMaterial : public Material {
public:
	MatteMaterial(const Vector3f& color);
	MatteMaterial(const std::shared_ptr<Texture2DC>& colorTexture);
	Vector3f getBRDF(const HitRecord& hitRecord,
					 const Vector3f& wi,
					 const Vector3f& wo) const override;
	Vector3f getDirection(const HitRecord& hitRecord, const Vector3f& wo) const override;

protected:
	std::shared_ptr<Texture2DC> color;
};
}  // namespace xd
#endif	// XD_RT_MATERIAL_H
