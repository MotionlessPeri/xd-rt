//
// Created by Frank on 2023/11/28.
//
#include "Sampler.h"
using namespace xd;
Sampler::Sampler(int samplePerPixel) : samplePerPixel(samplePerPixel) {}
void Sampler::setCurrentPixel(const Vector2i& pixel)
{
	currentPixel = pixel;
	currentSampleIndex = 0;
}

bool Sampler::startNextSample()
{
	++currentSampleIndex;
	return currentSampleIndex < samplePerPixel;
}
