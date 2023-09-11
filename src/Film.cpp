//
// Created by Frank on 2023/8/16.
//

#include "Film.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using namespace xd;

Film::Film(const Vector3f& center,
		   const Vector3f& right,
		   const Vector3f& up,
		   uint32_t width,
		   uint32_t height)
	: center(center),
	  right(right),
	  up(up),
	  width(width),
	  height(height),
	  image(width * height * channels, 0.f)
{
	towards = -up.cross(right).normalized();
}

Vector3f Film::getTopLeft() const
{
	return center - right + up;
}

void Film::addSample(const ColorRGB& color, const Vector2f& samplePos)
{
	// TODO: this method is not thread-safe. We may need a thread-safe version
	const auto index = getIndexFromSample(samplePos);
	image[channels * index] += color.x();
	image[channels * index + 1] += color.y();
	image[channels * index + 2] += color.z();
}

ColorRGB Film::getPixelColor(const Vector2f& pos)
{
	const auto index = getIndexFromSample(pos);
	return {image[channels * index], image[channels * index + 1], image[channels * index + 2]};
}

std::pair<Vector3f, Vector3f> Film::getDxDy() const
{
	return {2 * right / width, -2 * up / height};
}

uint32_t Film::getIndexFromSample(const Vector2f& sample) const
{
	// TODO: now we assign the left and top border to the pixel. Right and bottom border is not
	// belong to thepixel. This might be changed for a more convincing sample quality
	auto origin = getTopLeft();
	const double x = (2 * right * sample.x()).norm();
	const double y = (-2 * up * sample.y()).norm();
	const auto [dx, dy] = getDxDy();
	const auto row = std::floor(y / dy.norm());
	const auto col = std::floor(x / dx.norm());
	const auto index = row * width + col;
	return index;
}

Vector3f Film::getWorldCoordsFromSample(const Vector2f& sample) const
{
	auto origin = getTopLeft();
	const Vector3f x = 2 * right * sample.x();
	const Vector3f y = -2 * up * sample.y();
	return origin + x + y;
}

void Film::saveToFile(const std::string& fileName)
{
	if (fileName.substr(fileName.size() - 4) != ".hdr") {
		throw std::runtime_error{"invalid fileName.\n"};
	}
	const auto resCode = stbi_write_hdr(fileName.c_str(), width, height, channels, image.data());
	if (resCode == 0) {
		throw std::runtime_error{"save file failed.\n"};
	}
	return;
}
