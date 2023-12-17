//
// Created by Frank on 2023/12/1.
//
#include "SamplerIntegrator.h"
#include "Camera.h"
#include "Film.h"
#include "Light.h"
#include "Sampler.h"
#include "Scene.h"
#include "oneapi/tbb.h"
using namespace xd;

SamplerIntegrator::SamplerIntegrator(const std::shared_ptr<Sampler>& sampler) : sampler(sampler) {}

SamplerIntegrator::SamplerIntegrator(const IntegratorConfig& config,
									 const std::shared_ptr<Sampler>& sampler)
	: Integrator(config), sampler(sampler)
{
}
void SamplerIntegrator::render(const xd::Scene& scene)
{
	auto film = camera->getFilm();
	const auto resolution = film->getResolution();
	// For debug purpose. Make tbb run sequentially
	// oneapi::tbb::global_control
	// global_limit(oneapi::tbb::global_control::max_allowed_parallelism, 1);
	oneapi::tbb::parallel_for(
		tbb::blocked_range2d<int, int>{0, resolution.y(), 0, resolution.x()},
		[&](const tbb::blocked_range2d<int, int>& range) {
			const Vector2i topLeft{range.cols().begin(), range.rows().begin()};
			const Vector2i bottomRight{range.cols().end() - 1, range.rows().end() - 1};
			auto tile = film->getTile(topLeft, bottomRight);
			if (!tile)
				return;
			auto tileSampler = sampler->clone(topLeft.y() * resolution.x() + topLeft.x());
			for (const auto pixel : *tile) {
				tileSampler->setCurrentPixel(pixel);
				do {
					const Vector2f pixelSample = pixel.cast<float>() + tileSampler->sample2D();
					const auto primRay = camera->generateRay(pixelSample);
					tile->addSample(Li(primRay, scene, *tileSampler), pixelSample);
				} while (tileSampler->startNextSample());
			}
			film->mergeTileToFilm(std::move(tile));
		});
}
