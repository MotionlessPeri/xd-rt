//
// Created by Frank on 2023/12/16.
//

#ifndef XD_RT_PERFECTTRANSMISSIONMATERIAL_H
#define XD_RT_PERFECTTRANSMISSIONMATERIAL_H
#include "HitRecord.h"
#include "Material.h"
#include "bxdf/PerfectTransmission.h"
namespace xd {
class PerfectTransmissionMaterial : public Material {
public:
	PerfectTransmissionMaterial(float etaOutside, float etaInside);
	ColorRGB getBxDF(const HitRecord& primRec,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						Vector3f& wi) const override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const HitRecord& primRec,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									const Vector3f& wo,
									float& pdf) const override;
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
	static auto getTransform(const HitRecord& rec, bool flipNormal)
	{
		struct Ret {
			Matrix3f localToWorld;
			Matrix3f worldToLocal;
		} ret;
		const auto n = flipNormal ? -rec.n : rec.n;
		const auto& x = rec.dpdu;
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
