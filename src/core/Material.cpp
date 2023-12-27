//
// Created by Frank on 2023/12/22.
//
#include "Material.h"
#include "HitRecord.h"
#include "Texture.h"
using namespace xd;

ShadingDerivatives PhysicalPlausibleMaterial::getShadingGeometry(const LocalGeomParams& geom)
{
	if (!normal)
		return static_cast<ShadingDerivatives>(geom);
	const auto toGeomFrame = geom.getCurrentFrame();
	const auto n = normal->sample(geom.uv);
	ShadingDerivatives shading;
	const auto& geomDpdu = geom.derivatives.dpdu;
	const auto& geomDpdv = geom.derivatives.dpdv;
	// Note: we find shading.dpdu by using gram schimidt orthogonalization. If we wanna a more
	// accurate result, sampling normal map to get dndu and dndv directly might be better.
	shading.n = applyTransformToNormal(toGeomFrame, n);
	shading.dpdu = GramSchmidt(geomDpdu, shading.n);
	shading.dpdv = shading.n.cross(shading.dpdu).normalized() * geomDpdv.norm();
	return shading;
}
