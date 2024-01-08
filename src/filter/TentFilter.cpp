//
// Created by Frank on 2024/1/4.
//

#include "TentFilter.h"

#include <iostream>
using namespace xd;
TentFilter::TentFilter(WrapMode wrap_s, WrapMode wrap_t) : ImageFilter2D(wrap_s, wrap_t) {}

ColorRGBA TentFilter::filter(const std::shared_ptr<Image2D>& image, const Vector2f& pos) const
{
	// const auto floatingCoords = pos.cwiseProduct(image->getExtent().cast<float>());
	const Vector2f floatingCoords = pos.cwiseProduct(image->getExtent().cast<float>());
	const Vector2i intCoords = floatingCoords.cast<int>();
	float dx = floatingCoords.x() - intCoords.x() - 0.5f;
	float dy = floatingCoords.y() - intCoords.y() - 0.5f;
	int topLeftX = intCoords.x();
	if (dx < 0) {
		dx = 1 + dx;
		topLeftX -= 1;
	}
	int topLeftY = intCoords.y();
	if (dy < 0) {
		dy = 1 + dy;
		topLeftY -= 1;
	}
	std::array<ColorRGBA, 4> values;
	values[0] = getPixelValue(image, topLeftY, topLeftX);
	values[1] = getPixelValue(image, topLeftY, topLeftX + 1);
	values[2] = getPixelValue(image, topLeftY + 1, topLeftX);
	values[3] = getPixelValue(image, topLeftY + 1, topLeftX + 1);
	return (1 - dx) * (1 - dy) * values[0] + dx * (1 - dy) * values[1] + (1 - dx) * dy * values[2] +
		   dx * dy * values[3];
}
