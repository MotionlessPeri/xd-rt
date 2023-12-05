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
using namespace xd;
void DebugIntegrator::render(const Scene& scene)
{
	auto film = camera->getFilm();
	const auto resolution = film->getResolution();
	// For debug purpose. Make tbb run sequentially
	if (enableParallel) {
		oneapi::tbb::global_control global_limit(
			oneapi::tbb::global_control::max_allowed_parallelism, 1);
	}
	oneapi::tbb::parallel_for(
		tbb::blocked_range2d<int, int>{0, resolution.x(), 0, resolution.y()},
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.rows().begin(), range.cols().begin()};
			const Vector2i bottomRight{range.rows().end() - 1, range.cols().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);

			for (const auto& pixel : *tile) {
				if (pixel == debugBreakPixel) {
					// TODO: wrap the debug break function, making it cross-platform
					__debugbreak();
				}
				const Vector2f pixelSample = pixel.cast<float>() + Vector2f{0.5, 0.5};
				const auto primRay = camera->generateRay(pixelSample);
				HitRecord primRec;
				if (!scene.hit(primRay, primRec)) {
					const auto env = scene.getEnvironment();
					if (env) {
						tile->addSample(env->getRadiance(primRec, primRay.d), pixelSample);
					}
					continue;
				}
				primRec.buildFrames();
				const auto material = primRec.primitive->getMaterial();
				tile->addSample(getDebugResult(channel, primRec, primRay), pixelSample);
			}
			film->mergeTileToFilm(std::move(tile));
		});
}

ColorRGB DebugIntegrator::getDebugResult(DebugChannel channel,
										 const HitRecord& primRec,
										 const Ray& primRay)
{
	// TODO: consider change DebugIntegrator to template class and refactor this method to template
	// function using partial specialization to increase performance
	switch (channel) {
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
		default:
			return {0, 0, 0};
	}
}
