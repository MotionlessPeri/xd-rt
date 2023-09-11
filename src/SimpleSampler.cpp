//
// Created by Frank on 2023/8/16.
//
#include "Sampler.h"
using namespace xd;

SimpleSampler::SimpleSampler(uint32_t width, uint32_t height) : width(width), height(height) {}

std::vector<Vector2f> SimpleSampler::generateSamples()
{
	std::vector<Vector2f> res;
	res.reserve(width * height);
	const double dx = 1. / width;
	const double dy = 1. / height;
	const double halfDx = dx / 2;
	const double halfDy = dy / 2;
	for (auto row = 0; row < height; ++row) {
		for (auto col = 0; col < width; ++col) {
			res.emplace_back(halfDx + col * dx, halfDy + row * dy);
		}
	}
	return res;
}