//
// Created by Frank on 2024/1/4.
//

#include "NearestFilter.h"

namespace xd {
NearestFilter::NearestFilter(WrapMode mode, std::shared_ptr<Image2D> image)
	: Filter2D(mode, std::move(image))
{
}
ColorRGBA NearestFilter::filter(const Vector2f& pos) const
{
	const auto intCoords = pos.cast<uint32_t>();
	return getPixelValue(intCoords.y(), intCoords.x());
}
}  // namespace xd