//
// Created by Frank on 2023/11/29.
//
#include "Camera.h"
#include "Film.h"
#include "Integrator.h"
#include "Light.h"
#include "Primitive.h"
#include "Scene.h"
#include "oneapi/tbb.h"
#include "sampler/SimpleSampler.h"
using namespace xd;
void DebugIntegrator::render(const Scene& scene)
{
	auto film = camera->getFilm();
	const auto resolution = film->getResolution();
	auto sampler = std::make_shared<SimpleSampler>(1);
	oneapi::tbb::parallel_for(
		tbb::blocked_range2d<int, int>{0, resolution.x(), 0, resolution.y()},
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.rows().begin(), range.cols().begin()};
			const Vector2i bottomRight{range.rows().end() - 1, range.cols().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			auto tileSampler = sampler->clone(topLeft.y() * resolution.x() + topLeft.x());
			if (!tile)
				return;
			for (const auto& pixel : *tile) {
				sampler->setCurrentPixel(pixel);
				if (pixel == debugBreakPixel) {
					// TODO: wrap the debug break function, making it cross-platform
					__debugbreak();
				}
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.1, 0.1};
				const auto primRay = camera->generateRay(pixelSample);
				HitRecord primRec;
				if (!scene.hit(primRay, primRec)) {
					const auto env = scene.getEnvironment();
					if (env) {
						tile->addSample(env->getRadiance(primRec, primRay.d), pixelSample);
					}
					continue;
				}
				const auto material = primRec.primitive->getMaterial();
				tile->addSample(getDebugResult(channel, primRec, primRay, scene, *tileSampler),
								pixelSample);
			}
			film->mergeTileToFilm(std::move(tile));
		});
}

ColorRGB DebugIntegrator::getDebugResult(DebugChannel channel,
										 const HitRecord& primRec,
										 const Ray& primRay,
										 const Scene& scene,
										 Sampler& sampler) const
{
	// TODO: consider change DebugIntegrator to template class and refactor this method to template
	// function using partial specialization to increase performance
	switch (channel) {
		case DebugChannel::HIT:
			return {1, 0, 0};
		case DebugChannel::SHADOW_HIT: {
			HitRecord shadowRec;
			const auto wi = primRec.n;
			const auto shadowRay = primRec.spawnRay(wi);
			if (scene.hit(shadowRay, shadowRec)) {
				return {shadowRec.tHit * 1e9f, 1, 0};
			}
			else {
				return {0, 0, 1};
			}
		}
		case DebugChannel::HIT_T:
			return {primRec.tHit, 0, 0};
		case DebugChannel::POSITION:
			return primRec.p;
		case DebugChannel::NORMAL:
			return primRec.n + Vector3f{1, 1, 1};
		case DebugChannel::UV:
			return {primRec.uv.x(), primRec.uv.y(), 0};
		case DebugChannel::BXDF: {
			const auto wo = -primRay.d;
			const auto wi = reflected(wo, primRec.n);
			const auto material = primRec.primitive->getMaterial();
			return material->getBRDF(primRec, wo, wi);
		}
		case DebugChannel::SINGLE_RADIANCE: {
			const auto light = scene.getLights()[lightIndex];
			HitRecord shadowRec;
			Vector3f wi;
			return light->sampleRadiance(sampler.sample2D(), primRec, shadowRec, wi);
		}
		case DebugChannel::TOTAL_RADIANCE: {
			Vector3f radiance{0, 0, 0};
			for (const auto& light : scene.getLights()) {
				radiance += light->getRadiance(primRec, primRec.n);
			}
			return radiance;
		}
		case DebugChannel::LIGHT_PDF: {
			const auto light = scene.getLights()[lightIndex];
			HitRecord shadowRec;
			Vector3f wi;
			float pdf;
			light->sampleRadianceWithPdf(sampler.sample2D(), primRec, shadowRec, wi, pdf);
			return {pdf, 0, 0};
		}
		case DebugChannel::TEMP: {
#if 0
			const auto light = scene.getLights()[lightIndex];
			ColorRGB res{0, 0, 0};
			constexpr int cnt = 10;
			for (auto i : std::views::iota(0, cnt)) {
				HitRecord shadowRec;
				Vector3f wi;
				float pdf;
				const auto radiance =
					light->sampleRadianceWithPdf(sampler.sample2D(), primRec, shadowRec, wi, pdf);
				const auto brdf =
					primRec.primitive->getMaterial()->getBRDF(primRec, -primRay.d, wi);
				const auto cosTheta = std::clamp(primRec.n.dot(wi), 0.f, 1.f);
				res += (radiance.cwiseProduct(brdf) * cosTheta / pdf);
			}
			return res / cnt;
#else
			const auto light = scene.getLights()[lightIndex];
			const auto material = primRec.primitive->getMaterial();
			ColorRGB res{0, 0, 0};
			constexpr int cnt = 10;
			for (auto i : std::views::iota(0, cnt)) {
				HitRecord shadowRec;
				Vector3f wi;
				float pdf;
				const auto brdf =
					material->sampleBRDFWithPdf(sampler.sample2D(), primRec, -primRay.d, wi, pdf);
				const auto radiance = light->getRadiance(primRec, wi);
				const auto cosTheta = std::clamp(primRec.n.dot(wi), 0.f, 1.f);
				res += (radiance.cwiseProduct(brdf) * cosTheta / pdf);
			}
			return res / cnt;
#endif
		}
		default:
			return {0, 0, 0};
	}
}
