//
// Created by Frank on 2023/11/28.
//

#ifndef XD_RT_INTEGRATOR_H
#define XD_RT_INTEGRATOR_H
#include "CoreTypes.h"
namespace xd {
struct IntegratorConfig {
	bool showEnv = true;
};
class Integrator {
public:
	explicit Integrator() = default;
	explicit Integrator(const IntegratorConfig& config) : config(config) {}
	virtual void setCamera(const std::shared_ptr<Camera>& cam) { camera = cam; }
	virtual void render(const Scene& scene) = 0;

protected:
	IntegratorConfig config{};
	std::shared_ptr<Camera> camera;
};

/**
 * \brief Estimate light contribution for a given light using MIS
 * \param primRay the ray that hit the surface
 * \param primRec the local information of hit point. Contains the bxdf(material) info
 * \param uLight sample used by sampling light
 * \param light the light to be sampled
 * \param scene the whole scene, used for visibility test on shadow ray
 * \return the radiance returned in -primRay.d
 */
ColorRGB SampleLightContribution(const Ray& primRay,
								 const HitRecord& primRec,
								 const Vector2f& uLight,
								 const Light& light,
								 const Scene& scene);
/**
 * \brief Estimate material contribution for all lights using MIS
 * \param primRay the ray that hit the surface
 * \param primRec the local information of hit point. Contains the bxdf(material) info
 * \param uBxdf sample used by sampling bxdf
 * \param uChooseLight sample used by lightSampler to determine which light to sample
 * \param scene the whole scene, used for visibility test on shadow ray
 * \param lightSampler light sampler that use to choose which light to sample. If lightSampler is
 * nullptr, all lights will be sampled.
 * \param sampleSpecular if the method samples delta bxdf
 * \return the radiance returned in -primRay.d
 */
ColorRGB SampleMaterialContribution(const Ray& primRay,
									const HitRecord& primRec,
									const Vector2f& uBxdf,
									float uChooseLight,
									const Scene& scene,
									const LightSampler* lightSampler,
									bool sampleSpecular);
/**
 * \brief Estimate direct illumination using multiple important sampling between bxdf and light
 * \param primRay the ray that hit the surface
 * \param primRec the local information of hit point. Contains the bxdf(material) info
 * \param uLight sample used by sampling light
 * \param light the light to be sampled
 * \param uBxdf sample used by sampling bxdf
 * \param scene the whole scene, used for visibility test on shadow ray
 * \param lightSampler light sampler that use to choose which light to sample. If lightSampler is
 * nullptr, all lights will be sampled.
 * \param sampleSpecular if the method samples delta bxdf
 * \return the radiance returned in -primRay.d
 */
ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						float uChooseLight,
						const Scene& scene,
						const LightSampler* lightSampler,
						bool sampleSpecular);

}  // namespace xd
#endif	// XD_RT_INTEGRATOR_H
