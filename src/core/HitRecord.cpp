//
// Created by Frank on 2023/12/23.
//
#include "HitRecord.h"
#include <cassert>
#include "Primitive.h"
using namespace xd;

ShadingDerivatives::ShadingDerivatives(Vector3f n,
									   Vector3f dpdu,
									   Vector3f dpdv,
									   Vector3f dndu,
									   Vector3f dndv)
	: n(std::move(n)),
	  dpdu(std::move(dpdu)),
	  dpdv(std::move(dpdv)),
	  dndu(std::move(dndu)),
	  dndv(std::move(dndv))
{
}

ShadingDerivatives::ShadingDerivatives(const ShadingDerivatives& other, Vector3f shadingNormal)
	: n(std::move(shadingNormal))
{
	dpdu = GramSchmidt(other.dpdu, n);
	dpdv = n.cross(dpdu).normalized() * other.dpdv.norm();
	// TODO: cal dndu and dndv
}

void ShadingDerivatives::applyTransform(const Transform& transform)
{
	n = applyTransformToNormal(transform, n);
	dpdu = applyTransformToDirection(transform, dpdu);
	dpdv = applyTransformToDirection(transform, dpdv);
	dndu = applyTransformToDirection(transform, dndu);
	dndv = applyTransformToDirection(transform, dndv);
}

LocalGeomParams::LocalGeomParams(FrameCategory frame,
								 Vector3f p,
								 Vector2f uv,
								 Vector3f n,
								 Vector3f dpdu,
								 Vector3f dpdv,
								 Vector3f dndu,
								 Vector3f dndv)
	: frame(frame),
	  p(std::move(p)),
	  uv(std::move(uv)),
	  derivatives(std::move(n), std::move(dpdu), std::move(dpdv), std::move(dndu), std::move(dndv))
{
}

LocalGeomParams::LocalGeomParams(FrameCategory frame,
								 Vector3f p,
								 Vector2f uv,
								 ShadingDerivatives geom)
	: frame(frame), p(std::move(p)), uv(std::move(uv)), derivatives(std::move(geom))
{
}

LocalGeomParams::operator ShadingDerivatives() const
{
	return derivatives;
}

LocalGeomParams::operator TextureEvalContext() const
{
	TextureEvalContext res;
	res.p = p;
	res.uv = uv;
	res.n = derivatives.n;
	return res;
}

Transform LocalGeomParams::getCurrentFrame() const
{
	const auto& z = derivatives.n;
	const auto x = derivatives.dpdu.normalized();
	const auto y = coordSystem(derivatives.n, x);
	return Transform{buildFrameMatrix(x, y, z)};
}
void LocalGeomParams::setShadingGeometry(ShadingDerivatives geom)
{
	derivatives = std::move(geom);
}

void LocalGeomParams::applyTransform(FrameCategory frame,
									 const Transform& transform,
									 Vector3f* pError)
{
	this->frame = frame;
	p = applyTransformToPoint(transform, p, pError);
	derivatives.applyTransform(transform);
}

HitRecord::HitRecord(float t_hit) : tHit(t_hit) {}

HitRecord::HitRecord(float t_hit,
					 Vector3f p_error,
					 std::shared_ptr<const Primitive> primitive,
					 FrameCategory frame,
					 Vector3f p,
					 Vector2f uv,
					 Vector3f n,
					 Vector3f dpdu,
					 Vector3f dpdv,
					 Vector3f dndu,
					 Vector3f dndv)
	: tHit(t_hit),
	  primitive(std::move(primitive)),
	  geom(frame, p, uv, n, dpdu, dpdv, dndu, dndv),
	  pError(std::move(p_error))
{
}

void HitRecord::applyTransform(FrameCategory frame, const Transform& transform)
{
	geom.applyTransform(frame, transform, &pError);
}

Ray HitRecord::spawnRay(const Vector3f& w) const
{
	return {offsetRayOrigin(geom.p, pError, geom.derivatives.n, w), w};
}
const ShadingDerivatives& HitRecord::getShadingGeometry() const
{
	if (shadingDerivatives)
		return *shadingDerivatives;
	const auto material = getPhysicalPlausibleMaterial();
	if (!material)
		shadingDerivatives = geom.derivatives;
	else {
		shadingDerivatives = material->getShadingGeometry(geom);
	}
	return *shadingDerivatives;
}

LocalGeomParams HitRecord::getShadingGeomParams() const
{
	return {geom.frame, geom.p, geom.uv, getShadingGeometry()};
}

ColorRGB HitRecord::getBxDF(const Vector3f& wo, const Vector3f& wi) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->getBxDF(getShadingGeomParams(), wo, wi);
}

SampleBxDFResult HitRecord::sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->sampleBxDF(uSample, getShadingGeomParams(), wo);
}

SampleBxDFPdfResult HitRecord::sampleBxDFPdf(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->sampleBxDFWithPdf(uSample, getShadingGeomParams(), wo);
}

Vector3f HitRecord::sampleMaterialWi(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->sampleDirection(uSample, getShadingGeomParams(), wo);
}

SampleDirPdfResult HitRecord::sampleMaterialWiPdf(const Vector2f& uSample, const Vector3f& wo) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->sampleDirectionWithPdf(uSample, getShadingGeomParams(), wo);
}

float HitRecord::getMaterialPdf(const Vector3f& wo) const
{
	const auto mtl = getPhysicalPlausibleMaterial();
#if _DEBUG
	if (!mtl)
		assert(false);
#endif
	return mtl->getPdf(getShadingGeomParams(), wo);
}

std::shared_ptr<PhysicalPlausibleMaterial> HitRecord::getPhysicalPlausibleMaterial() const
{
	// TODO: refactor if rtti here is a bottle neck
	if (!primitive)
		assert(false);
	return std::dynamic_pointer_cast<PhysicalPlausibleMaterial>(primitive->material);
}
