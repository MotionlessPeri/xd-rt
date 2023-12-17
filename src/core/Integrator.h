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
 * Esitimate direct illumination using multiple important sampling between bxdf and light
 * @param primRec the local information of hit point. Contains the bxdf(material) info
 * @param uLight sample used by sampling light
 * @param light the light to be sampled
 * @param uBxdf sample used by sampling bxdf
 * @param scene the whole scene, used for visibility test on shadow ray
 * @return the radiance returned in -primRay.d
 */
ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						const Scene& scene);

}  // namespace xd
#endif	// XD_RT_INTEGRATOR_H
