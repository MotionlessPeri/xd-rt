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

class SamplerIntegrator : public Integrator {
public:
	explicit SamplerIntegrator(const std::shared_ptr<Sampler>& sampler);

	SamplerIntegrator(const IntegratorConfig& config, const std::shared_ptr<Sampler>& sampler);

	virtual ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) = 0;
	void render(const Scene& scene) override;

protected:
	std::shared_ptr<Sampler> sampler;
};

enum class DebugChannel {
	HIT,				// if primary ray hit, {1, 0, 0} will return
	SHADOW_HIT,			// if shadow ray hit sth, {t, 1, 0} will return; else {0, 0, 1} will return
	HIT_T,				//
	POSITION,			//
	NORMAL,				// if hit, geomNormal + Vector3f{1, 1, 1} will return
	UV,					//
	BXDF,				//
	SINGLE_IRRADIANCE,	// radiance emitted by scene.lights[lightIndex]
	TOTAL_IRRADIANCE,	// radiance emitted by all lights in the scene
	LIGHT_PDF,			// sample direction and pdf using scene.lights[lightIndes]
	SAMPLE_BRDF_RADIANCE,	// the radiance calculated by sampling brdf
	SAMPLE_LIGHT_RADIANCE,	// the raidance calculated by sampling light
	TEMP					// use for temporary debug only
};
class DebugIntegrator : public Integrator {
public:
	explicit DebugIntegrator() = default;

	explicit DebugIntegrator(const IntegratorConfig& config) : Integrator(config) {}

	void render(const Scene& scene) override;
	void setDebugChannel(DebugChannel debugChannel) { channel = debugChannel; }
	void setDebugBreakPixel(const Vector2i& pixel) { debugBreakPixel = pixel; }
	void setLightIndex(uint32_t index) { lightIndex = index; }

protected:
	ColorRGB getDebugResult(DebugChannel channel,
							const HitRecord& primRec,
							const Ray& primRay,
							const Scene& scene,
							Sampler& sampler) const;
	DebugChannel channel = DebugChannel::HIT_T;
	Vector2i debugBreakPixel{-1, -1};
	uint32_t lightIndex = 0;
};

class MIDirectIntegrator : public SamplerIntegrator {
public:
	explicit MIDirectIntegrator(const std::shared_ptr<Sampler>& sampler);
	MIDirectIntegrator(const IntegratorConfig& config, const std::shared_ptr<Sampler>& sampler);
	void render(const Scene& scene) override;
	ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) override;
};

class PathIntegrator : public SamplerIntegrator {
public:
	PathIntegrator(const std::shared_ptr<Sampler>& sampler, int maxDepth);
	PathIntegrator(const IntegratorConfig& config,
				   const std::shared_ptr<Sampler>& sampler,
				   int max_depth);
	ColorRGB Li(const Ray& r, const Scene& scene, Sampler& sampler) override;

protected:
	int maxDepth;
};
}  // namespace xd
#endif	// XD_RT_INTEGRATOR_H
