//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_DEBUGINTEGRATOR_H
#define XD_RT_DEBUGINTEGRATOR_H
#include "Integrator.h"
namespace xd {
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
}  // namespace xd
#endif	// XD_RT_DEBUGINTEGRATOR_H
