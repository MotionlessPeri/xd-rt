//
// Created by Frank on 2024/1/4.
//

#include "TentFilter.h"
using namespace xd;
TentFilter::TentFilter(WrapMode mode, std::shared_ptr<Image2D> image)
	: Filter2D(mode, std::move(image))
{
}

ColorRGBA TentFilter::filter(const Vector2f& pos) const
{
	const auto intCoords = pos.cast<int>();
	const float dx = pos.x() - intCoords.x() - 0.5f;
	const float dy = pos.y() - intCoords.y() - 0.5f;
	const int topLeftX = dx < 0 ? intCoords.x() - 1 : intCoords.x();
	const int topLeftY = dy < 0 ? intCoords.y() - 1 : intCoords.y();
	std::array<ColorRGBA, 4> values;
	values[0] = getPixelValue(topLeftY, topLeftX);
	values[1] = getPixelValue(topLeftY, topLeftX + 1);
	values[2] = getPixelValue(topLeftY + 1, topLeftX);
	values[3] = getPixelValue(topLeftY + 1, topLeftX + 1);
	return (1 - dx) * (1 - dy) * values[0] + dx * (1 - dy) * values[1] + (1 - dx) * dy * values[2] +
		   dx * dy * values[3];
}
