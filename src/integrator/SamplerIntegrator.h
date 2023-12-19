//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_SAMPLERINTEGRATOR_H
#define XD_RT_SAMPLERINTEGRATOR_H
#include "Integrator.h"
namespace xd {
class SamplerIntegrator : public Integrator {
public:
	explicit SamplerIntegrator(std::shared_ptr<Sampler> sampler);

	SamplerIntegrator(const IntegratorConfig& config, std::shared_ptr<Sampler> sampler);

	virtual ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) = 0;
	void render(const Scene& scene) override;

protected:
	std::shared_ptr<Sampler> sampler;
};
}  // namespace xd
#endif	// XD_RT_SAMPLERINTEGRATOR_H
