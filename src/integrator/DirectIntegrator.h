//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_DIRECTINTEGRATOR_H
#define XD_RT_DIRECTINTEGRATOR_H
#include "SamplerIntegrator.h"
namespace xd {
class MIDirectIntegrator : public SamplerIntegrator {
public:
	explicit MIDirectIntegrator(const std::shared_ptr<Sampler>& sampler);
	MIDirectIntegrator(const IntegratorConfig& config, const std::shared_ptr<Sampler>& sampler);
	void render(const Scene& scene) override;
	ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) override;
};
}  // namespace xd
#endif	// XD_RT_DIRECTINTEGRATOR_H
