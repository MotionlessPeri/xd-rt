//
// Created by Frank on 2023/8/26.
//

#ifndef XD_RT_MATERIAL_H
#define XD_RT_MATERIAL_H
#include "BxDF.h"
#include "CoreTypes.h"
#include "MathTypes.h"
#include "texture/TextureTypes.h"
namespace xd {
/**
 * Material is an aggregate of brdfs
 */
class Material {
public:
	virtual ~Material() = default;
};

class PhysicalPlausibleMaterial : public Material {
public:
	PhysicalPlausibleMaterial() = default;

	explicit PhysicalPlausibleMaterial(std::shared_ptr<Texture> normal);

	/**
	 * get bxdf value
	 * @param shadingGeom the geometric properties around hit point
	 * @param wo the incoming direction. Wi must lies in world frame.
	 * @param wi the outgoing direction. Wo must lies in world frame.
	 * @return the bxdf value respected to wi and wo
	 */
	virtual ColorRGB getBxDF(const LocalGeomParams& shadingGeom,
							 const Vector3f& wo,
							 const Vector3f& wi) const = 0;
	/**
	 * \brief sample the bxdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param shadingGeom the local presentation of the hit point
	 * \return the bxdf value and wi's direction
	 */
	virtual SampleBxDFResult sampleBxDF(const Vector2f& uSample,
										const LocalGeomParams& shadingGeom,
										const Vector3f& wo) const = 0;
	/**
	 * \brief sample the bxdf of a given point
	 * \param uSample the sampled point used to sample wo
	 * \param shadingGeom the local presentation of the hit point
	 * \param wo the incoming direction. Wo must lies in world frame.
	 * \return the bxdf value, wi's direction and pdf
	 */
	virtual SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
												  const LocalGeomParams& shadingGeom,
												  const Vector3f& wo) const = 0;
	/**
	 * \brief get pdf value of given wo
	 * \param shadingGeom the local representation of hit point
	 * \param wo the outgoing direction. Wo must lies in world frame
	 * \return the pdf value associate to wo
	 */
	virtual float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const = 0;
	/**
	 * \brief sample wo's direction according to wi and bxdf's distribution
	 * \param uSample the sampled point used for sampling direction
	 * \param wo outgoing direction
	 * \param shadingGeom the local representation of hit point
	 * \return wo's direction. Note that wo is in world frame
	 */
	virtual Vector3f sampleDirection(const Vector2f& uSample,
									 const LocalGeomParams& shadingGeom,
									 const Vector3f& wo) const = 0;
	virtual SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
													  const LocalGeomParams& shadingGeom,
													  const Vector3f& wo) const = 0;
	virtual bool isDelta() const = 0;
	virtual ShadingDerivatives getShadingGeometry(const LocalGeomParams& geom);

protected:
	std::shared_ptr<Texture> normal = nullptr;
};
}  // namespace xd
#endif	// XD_RT_MATERIAL_H
