//
// Created by Frank on 2023/11/28.
//

#ifndef XD_RT_INTEGRATOR_H
#define XD_RT_INTEGRATOR_H
#include "CoreTypes.h"
namespace xd {

class Integrator {
public:
	virtual void setCamera(const std::shared_ptr<Camera>& cam) { camera = cam; }
	virtual void render(const Scene& scene) = 0;

protected:
	std::shared_ptr<Camera> camera;
};

/**
 * Esitimate direct illumination using multiple important sampling between bxdf and light
 * @param primRec the local information of hit point. Contains the bxdf(material) info
 * @param uLight sample used by sampling light
 * @param light the light to be sampled
 * @param uBxdf sample used by sampling bxdf
 * @param scene the whole scene, used for visibility test on shadow ray
 * @return
 */
ColorRGB EstimateDirect(const Ray& primRay,
						const HitRecord& primRec,
						const Vector2f& uLight,
						const Light& light,
						const Vector2f& uBxdf,
						const Scene& scene);

class SamplerIntegrator : public Integrator {
public:
	explicit SamplerIntegrator(const std::shared_ptr<Sampler>& sampler) : sampler(sampler) {}
	virtual ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) = 0;
	void render(const Scene& scene) override;

protected:
	std::shared_ptr<Sampler> sampler;
};

enum class DebugChannel { HIT_T, POSITION, NORMAL, UV, BXDF };
class DebugIntegrator : public Integrator {
public:
	void render(const Scene& scene) override;
	void setDebugChannel(DebugChannel debugChannel) { channel = debugChannel; }
	void setDebugBreakPixel(const Vector2i& pixel) { debugBreakPixel = pixel; }
	void setEnableParallel(bool enable) { enableParallel = enable; }

protected:
	static ColorRGB getDebugResult(DebugChannel channel,
								   const HitRecord& primRec,
								   const Ray& primRay);
	DebugChannel channel = DebugChannel::HIT_T;
	Vector2i debugBreakPixel{-1, -1};
	bool enableParallel = true;
};

class MIDirectIntegrator : public SamplerIntegrator {
public:
	explicit MIDirectIntegrator(const std::shared_ptr<Sampler>& sampler);
	void render(const Scene& scene) override;
	ColorRGB Li(const Ray& ray, const Scene& scene, Sampler& sampler) override;
};

class PathIntegrator : public SamplerIntegrator {
public:
	PathIntegrator(const std::shared_ptr<Sampler>& sampler, int maxDepth);
	ColorRGB Li(const Ray& r, const Scene& scene, Sampler& sampler) override;

protected:
	int maxDepth;
};
}  // namespace xd
#endif	// XD_RT_INTEGRATOR_H
