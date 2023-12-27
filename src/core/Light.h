//
// Created by Frank on 2023/8/20.
//

#ifndef XD_RT_LIGHT_H
#define XD_RT_LIGHT_H

#include "CoreTypes.h"
#include "MathTypes.h"
namespace xd {
class Light {
public:
	explicit Light(uint32_t numSamples) : numSamples(numSamples) {}
	virtual ~Light() = default;

	struct SampleDirectionPdfResult {
		SampleDirectionPdfResult() = default;
		SampleDirectionPdfResult(const SampleDirectionPdfResult& other) = default;
		SampleDirectionPdfResult(SampleDirectionPdfResult&& other) noexcept = default;
		SampleDirectionPdfResult& operator=(const SampleDirectionPdfResult& other) = default;
		SampleDirectionPdfResult& operator=(SampleDirectionPdfResult&& other) noexcept = default;
		SampleDirectionPdfResult(Vector3f geomToLight, float pdf)
			: geomToLight(std::move(geomToLight)), pdf(pdf)
		{
		}
		~SampleDirectionPdfResult() = default;
		Vector3f geomToLight;
		float pdf;
	};
	struct SampleRadianceResult {
		SampleRadianceResult() = default;
		SampleRadianceResult(const SampleRadianceResult& other) = default;
		SampleRadianceResult(SampleRadianceResult&& other) noexcept = default;
		SampleRadianceResult& operator=(const SampleRadianceResult& other) = default;
		SampleRadianceResult& operator=(SampleRadianceResult&& other) noexcept = default;
		SampleRadianceResult(Vector3f geomToLight, ColorRGB radiance)
			: geomToLight(std::move(geomToLight)), radiance(std::move(radiance))
		{
		}
		~SampleRadianceResult() = default;
		Vector3f geomToLight;
		ColorRGB radiance;
	};
	struct SampleRadiancePdfResult : SampleRadianceResult {
		SampleRadiancePdfResult() = default;
		SampleRadiancePdfResult(const SampleRadiancePdfResult& other) = default;
		SampleRadiancePdfResult(SampleRadiancePdfResult&& other) noexcept = default;
		SampleRadiancePdfResult& operator=(const SampleRadiancePdfResult& other) = default;
		SampleRadiancePdfResult& operator=(SampleRadiancePdfResult&& other) noexcept = default;
		SampleRadiancePdfResult(Vector3f geomToLight, ColorRGB radiance, float pdf)
			: SampleRadianceResult(std::move(geomToLight), std::move(radiance)), pdf(pdf)
		{
		}
		SampleRadiancePdfResult(const SampleRadianceResult& other, float pdf)
			: SampleRadianceResult(other), pdf(pdf)
		{
		}
		SampleRadiancePdfResult(SampleRadianceResult&& other, float pdf)
			: SampleRadianceResult(other), pdf(pdf)
		{
		}
		SampleRadiancePdfResult(const SampleDirectionPdfResult& dirPdfRes, ColorRGB radiance)
			: SampleRadianceResult(dirPdfRes.geomToLight, std::move(radiance)), pdf(dirPdfRes.pdf)
		{
		}
		SampleRadiancePdfResult(SampleDirectionPdfResult&& dirPdfRes, ColorRGB radiance)
			: SampleRadianceResult(std::move(dirPdfRes.geomToLight), std::move(radiance)),
			  pdf(dirPdfRes.pdf)
		{
		}
		~SampleRadiancePdfResult() = default;
		float pdf = 0.f;
	};
	/**
	 * \brief sample a direction from primRec.p to the light
	 * \param uSample the sample used to sample light direction
	 * \return a structure includes a vector from shadingGeom.p to Light
	 */
	virtual Vector3f sampleDirection(const Vector2f& uSample,
									 const LocalGeomParams& shadingGeom) const = 0;
	/**
	 * \brief sample a direction from primRec.p to the light, return pdf as well.
	 * \param uSample the sample used to sample light direction.
	 * \param shadingGeom the local presentation of the hit point.
	 * \return a structure includes a vector from shadingGeom.p to Light and pdf.
	 */
	virtual SampleDirectionPdfResult sampleDirectionWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const = 0;
	/**
	 * \brief return the radiance of a point in a certain direction.
	 * \param shadingGeom the local presentation of the hit point.
	 * \param wi the light outgoing direction. Points from primRec.p to the light.
	 * \return the radiance reach the given point in direction wi.
	 */
	virtual ColorRGB getRadiance(const LocalGeomParams& shadingGeom, const Vector3f& wi) const = 0;
	/**
	 * \brief sample a direction and return the radiance emitted in that direction.
	 * \param uSample the sample used to sample light direction.
	 * \param shadingGeom the local presentation of the hit point.
	 * \return a structure includes a vector from shadingGeom.p to Light and light radiance.
	 */
	virtual SampleRadianceResult sampleRadiance(const Vector2f& uSample,
												const LocalGeomParams& shadingGeom) const = 0;
	/**
	 * \brief sample a direction and return the radiance emitted in that direction. Pdf is also
	 * returned.
	 * \param uSample the sample used to sample light direction.
	 * \param shadingGeom the local presentation of the hit point.
	 * \return a structure includes a vector from shadingGeom.p to Light, light radiance and pdf.
	 */
	virtual SampleRadiancePdfResult sampleRadianceWithPdf(
		const Vector2f& uSample,
		const LocalGeomParams& shadingGeom) const = 0;
	virtual bool isDelta() const = 0;
	virtual bool isInfinite() const = 0;
	virtual float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const = 0;

	uint32_t getNumSamples() const { return numSamples; }

protected:
	uint32_t numSamples;
};

}  // namespace xd
#endif	// XD_RT_LIGHT_H
