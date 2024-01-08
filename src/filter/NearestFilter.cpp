//
// Created by Frank on 2024/1/4.
//

#include "NearestFilter.h"
using namespace xd;
NearestFilter::NearestFilter(WrapMode wrap_s, WrapMode wrap_t) : ImageFilter2D(wrap_s, wrap_t) {}

ColorRGBA NearestFilter::filter(const std::shared_ptr<Image2D>& image, const Vector2f& pos) const
{
	const Vector2i intCoords = pos.cwiseProduct(image->getExtent().cast<float>()).cast<int>();
	return getPixelValue(image, intCoords.y(), intCoords.x());
}