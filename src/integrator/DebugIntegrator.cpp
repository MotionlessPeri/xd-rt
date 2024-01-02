//
// Created by Frank on 2023/11/29.
//
#include "DebugIntegrator.h"
#include "Camera.h"
#include "Film.h"
#include "Light.h"
#include "Primitive.h"
#include "Scene.h"
#include "material/PerfectFresnelMaterial.h"
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
						tile->addSample(env->getRadiance(primRec.geom, primRay.d), pixelSample);
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
			const auto wi = primRec.getShadingGeomParams().derivatives.n;
			const auto shadowRay = primRec.spawnRay(wi);
			if (scene.hit(shadowRay, shadowRec)) {
				return {shadowRec.tHit, 1, 0};
			}
			else {
				return {0, 0, 1};
			}
		}
		case DebugChannel::HIT_T:
			return {primRec.tHit, 0, 0};
		case DebugChannel::POSITION:
			return primRec.geom.p;
		case DebugChannel::GEOM_NORMAL:
			return primRec.geom.derivatives.n + Vector3f{1, 1, 1};
		case DebugChannel::SHADING_NORMAL: {
			const auto shadingGeom = primRec.getShadingGeomParams();
			return shadingGeom.derivatives.n + Vector3f{1, 1, 1};
		}
		case DebugChannel::UV:
			return {primRec.geom.uv.x(), primRec.geom.uv.y(), 0};
		case DebugChannel::BXDF: {
			const auto wo = -primRay.d;
			const auto wi = reflected(wo, primRec.geom.derivatives.n);
			const auto material = primRec.primitive->getMaterial();
			return primRec.getBxDF(wo, wi);
		}
		case DebugChannel::SINGLE_IRRADIANCE: {
			const auto light = scene.getLights()[lightIndex];
			ColorRGB res{0, 0, 0};
			constexpr uint32_t SAMPLE_CNT = 10u;
			for ([[maybe_unused]] auto i : std::views::iota(0u, SAMPLE_CNT)) {
				HitRecord shadowRec;
				Vector3f wi;
				res += light->sampleRadiance(sampler.sample2D(), primRec.getShadingGeomParams())
						   .radiance;
			}
			return res / SAMPLE_CNT;
		}
		case DebugChannel::TOTAL_IRRADIANCE: {
			Vector3f radiance{0, 0, 0};
			constexpr uint32_t SAMPLE_CNT = 10u;
			for ([[maybe_unused]] auto i : std::views::iota(0u, SAMPLE_CNT)) {
				for (const auto& light : scene.getLights()) {
					HitRecord shadowRec;
					Vector3f wi;
					radiance +=
						light->sampleRadiance(sampler.sample2D(), primRec.getShadingGeomParams())
							.radiance;
				}
			}

			return radiance / SAMPLE_CNT;
		}
		case DebugChannel::LIGHT_PDF: {
			const auto light = scene.getLights()[lightIndex];
			HitRecord shadowRec;
			return {light->sampleRadianceWithPdf(sampler.sample2D(), primRec.getShadingGeomParams())
						.pdf,
					0, 0};
		}
		case DebugChannel::SAMPLE_BRDF_RADIANCE: {
			const auto light = scene.getLights()[lightIndex];
			const auto material = primRec.primitive->getMaterial();
			ColorRGB res{0, 0, 0};
			constexpr int cnt = 10;
			for ([[maybe_unused]] auto i : std::views::iota(0, cnt)) {
				const auto shadingGeomParams = primRec.getShadingGeomParams();
				const auto brdfPdf = primRec.sampleBxDFPdf(sampler.sample2D(), -primRay.d);
				const auto radiance = light->getRadiance(shadingGeomParams, brdfPdf.dir);
				const auto cosTheta =
					std::clamp(shadingGeomParams.derivatives.n.dot(brdfPdf.dir), 0.f, 1.f);
				res += (radiance.cwiseProduct(brdfPdf.bxdf) * cosTheta / brdfPdf.pdf);
			}
			return res / cnt;
		}
		case DebugChannel::SAMPLE_LIGHT_RADIANCE: {
			const auto light = scene.getLights()[lightIndex];
			ColorRGB res{0, 0, 0};
			constexpr int cnt = 10;
			for ([[maybe_unused]] auto i : std::views::iota(0, cnt)) {
				const auto shadingGeom = primRec.getShadingGeomParams();
				const auto radianceDirPdf =
					light->sampleRadianceWithPdf(sampler.sample2D(), shadingGeom);
				const auto wi = radianceDirPdf.geomToLight.normalized();
				const auto brdf = primRec.getBxDF(-primRay.d, wi);
				const auto cosTheta = std::clamp(shadingGeom.derivatives.n.dot(wi), 0.f, 1.f);
				res += (radianceDirPdf.radiance.cwiseProduct(brdf) * cosTheta / radianceDirPdf.pdf);
			}
			return res / cnt;
		}
		case DebugChannel::TEMP: {
			const auto material = std::dynamic_pointer_cast<PerfectFresnelMaterial>(
				primRec.getPhysicalPlausibleMaterial());
			if (!material)
				__debugbreak();
			const auto shadingGeom = primRec.getShadingGeomParams();
			// const auto fresnelRes =
			//	material->getFresnel(sampler.sample2D(), shadingGeom, -primRay.d);
			// return {fresnelRes.fresnel, fresnelRes.reflect ? 1.f : 0.f, 0};
			const auto fresnelRes =
				material->sampleBxDFWithPdf(sampler.sample2D(), shadingGeom, -primRay.d);
			return {fresnelRes.pdf, 0, 0};
		}

		default:
			return {0, 0, 0};
	}
}
