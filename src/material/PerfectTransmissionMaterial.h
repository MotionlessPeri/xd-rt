//
// Created by Frank on 2023/12/16.
//

#ifndef XD_RT_PERFECTTRANSMISSIONMATERIAL_H
#define XD_RT_PERFECTTRANSMISSIONMATERIAL_H
#include "HitRecord.h"
#include "Material.h"
#include "bxdf/PerfectTransmission.h"
namespace xd {
class PerfectTransmissionMaterial : public PhysicalPlausibleMaterial {
public:
	PerfectTransmissionMaterial(float etaOutside, float etaInside);
	ColorRGB getBxDF(const LocalGeomParams& shadingGeom,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	SampleBxDFResult sampleBxDF(const Vector2f& uSample,
								const LocalGeomParams& shadingGeom,
								const Vector3f& wo) const override;
	SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
										  const LocalGeomParams& shadingGeom,
										  const Vector3f& wo) const override;
	float getPdf(const LocalGeomParams& shadingGeom, const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const LocalGeomParams& shadingGeom,
							 const Vector3f& wo) const override;
	SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
											  const LocalGeomParams& shadingGeom,
											  const Vector3f& wo) const override;
	bool isDelta() const override;

protected:
	auto chooseBtdf(const Vector3f& n, const Vector3f& wo) const
	{
		struct Ret {
			const std::unique_ptr<PerfectTransmission>& btdf;
			bool flipNormal;
		};
		if (n.dot(wo) > 0) {
			// from outside to inside
			return Ret{outToIn, false};
		}
		else {
			return Ret{inToOut, true};
		}
	}
	static auto getTransform(const LocalGeomParams& shadingGeom, bool flipNormal)
	{
		struct Ret {
			Transform localToWorld;
			Transform worldToLocal;
		} ret;
		const auto n = flipNormal ? -shadingGeom.derivatives.n : shadingGeom.derivatives.n;
		const auto& x = shadingGeom.derivatives.dpdu;
		const auto y = coordSystem(n, x);
		ret.localToWorld = buildFrameMatrix(x, y, n);
		ret.worldToLocal = ret.localToWorld.inverse();
		return ret;
	}
	std::unique_ptr<PerfectTransmission> inToOut;
	std::unique_ptr<PerfectTransmission> outToIn;
};
}  // namespace xd
#endif	// XD_RT_PERFECTTRANSMISSIONMATERIAL_H
