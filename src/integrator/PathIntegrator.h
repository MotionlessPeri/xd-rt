//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PATHINTEGRATOR_H
#define XD_RT_PATHINTEGRATOR_H
#include "SamplerIntegrator.h"
namespace xd {
class PathIntegrator : public SamplerIntegrator {
public:
	PathIntegrator(const std::shared_ptr<Sampler>& sampler, int maxDepth = 8);
	PathIntegrator(const IntegratorConfig& config,
				   const std::shared_ptr<Sampler>& sampler,
				   int max_depth);
	ColorRGB Li(const Ray& r, const Scene& scene, Sampler& sampler) override;
	void preProcess(const Scene& scene) override;

protected:
	int maxDepth{8};
	std::shared_ptr<LightSampler> lightSampler{nullptr};
};
}  // namespace xd
#endif	// XD_RT_PATHINTEGRATOR_H
