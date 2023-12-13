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
	  normedRight(right.normalized()),
	  up(up),
	  normedUp(up.normalized()),
	  resolution(width, height),
	  towards(normedUp.cross(normedRight)),
	  pixelExtent(right.norm() * 2 / resolution.x(), up.norm() * 2 / resolution.y()),
	  pixels(width * height),
	  croppedTopLeft(0, 0),
	  croppedBottomRight(width - 1, height - 1)
{
}

Film::Film(const Vector3f& center,
		   const Vector3f& right,
		   const Vector3f& up,
		   uint32_t width,
		   uint32_t height,
		   const Vector2i& croppedTopLeft,
		   const Vector2i& croppedBottomRight)
	: center(center),
	  right(right),
	  normedRight(right.normalized()),
	  up(up),
	  normedUp(up.normalized()),
	  resolution(width, height),
	  towards(normedUp.cross(normedRight)),
	  pixelExtent(right.norm() * 2 / resolution.x(), up.norm() * 2 / resolution.y()),
	  pixels(width * height),
	  croppedTopLeft(std::max(0, croppedTopLeft.x()), std::max(0, croppedTopLeft.y())),
	  croppedBottomRight(std::min<int>(width - 1, croppedBottomRight.x()),
						 std::min<int>(height - 1, croppedBottomRight.y()))
{
}

Film::Film(const Film& other)
	: center{other.center},
	  right{other.right},
	  normedRight{other.normedRight},
	  up{other.up},
	  normedUp{other.normedUp},
	  towards{other.towards},
	  resolution{other.resolution},
	  pixelExtent{other.pixelExtent},
	  pixels{other.pixels}
{
}

Film::Film(Film&& other) noexcept
	: center{std::move(other.center)},
	  right{std::move(other.right)},
	  normedRight{std::move(other.normedRight)},
	  up{std::move(other.up)},
	  normedUp{std::move(other.normedUp)},
	  towards{std::move(other.towards)},
	  resolution{std::move(other.resolution)},
	  pixelExtent{std::move(other.pixelExtent)},
	  pixels{std::move(other.pixels)}
{
}

Film& Film::operator=(const Film& other)
{
	if (this == &other)
		return *this;
	center = other.center;
	right = other.right;
	normedRight = other.normedRight;
	up = other.up;
	normedUp = other.normedUp;
	towards = other.towards;
	resolution = other.resolution;
	pixelExtent = other.pixelExtent;
	pixels = other.pixels;
	return *this;
}

Film& Film::operator=(Film&& other) noexcept
{
	if (this == &other)
		return *this;
	center = std::move(other.center);
	right = std::move(other.right);
	normedRight = std::move(other.normedRight);
	up = std::move(other.up);
	normedUp = std::move(other.normedUp);
	towards = std::move(other.towards);
	resolution = std::move(other.resolution);
	pixelExtent = std::move(other.pixelExtent);
	pixels = std::move(other.pixels);
	return *this;
}

FilmTile::FilmTile(const Vector2i& topLeft, const Vector2i& bottomRight)
	: topLeft(topLeft),
	  bottomRight(bottomRight),
	  tileExtent(bottomRight - topLeft + Vector2i{1, 1}),
	  tilePixels(tileExtent.x() * tileExtent.y())
{
}

std::unique_ptr<FilmTile> Film::getTile(const Vector2i& topLeft, const Vector2i& bottomRight) const
{
	// TODO: add filter
	const Vector2i tileTopLeft{std::max<int>(croppedTopLeft.x(), topLeft.x()),
							   std::max<int>(croppedTopLeft.y(), topLeft.y())};
	const Vector2i tileBottomRight{std::min<int>(croppedBottomRight.x(), bottomRight.x()),
								   std::min<int>(croppedBottomRight.y(), bottomRight.y())};
	if (tileTopLeft.cwiseGreater(tileBottomRight).any())
		return nullptr;
	return std::make_unique<FilmTile>(tileTopLeft, tileBottomRight);
}
void Film::mergeTileToFilm(std::unique_ptr<FilmTile> tile)
{
	std::lock_guard<std::mutex> lg(imageWriteLock);
	for (const auto pixel : *tile) {
		const Vector2f centerCoord{pixel.x() + 0.5f, pixel.y() + 0.5f};
		const auto tileIdx = tile->getIndexFromSample(centerCoord);
		const auto& tilePixel = tile->tilePixels[tileIdx];
		const auto filmIdx = pixel.y() * resolution.x() + pixel.x();
		auto& filmPixel = pixels[filmIdx];
		filmPixel.weight += tilePixel.weight;
		filmPixel.energy += tilePixel.energy;
	}
}

Vector3f Film::getTopLeft() const
{
	return center - right + up;
}

ColorRGB Film::getPixelColor(const Vector2f& pos)
{
	const auto index = getIndexFromSample(pos);
	return getPixelColor(index);
}

ColorRGB Film::getPixelColor(int index)
{
	const auto& tilePixel = pixels[index];
	constexpr float eps = 1e-5;
	if (std::fabs(tilePixel.weight) < eps) {
		return {0, 0, 0};
	}
	else
		return pixels[index].energy / pixels[index].weight;
}

uint32_t Film::getIndexFromSample(const Vector2f& sample) const
{
	const auto col = (int)sample.x();
	const auto row = (int)sample.y();
	return getIndexFromRowCol(row, col);
}

Vector3f Film::getWorldCoordsFromSample(const Vector2f& sample) const
{
	auto origin = getTopLeft();
	const Vector3f x = pixelExtent.x() * sample.x() * normedRight;
	const Vector3f y = pixelExtent.y() * sample.y() * normedUp;
	return origin + x - y;
}

void Film::saveToFile(const std::string& fileName, const Film::SaveOptions& options)
{
	if (fileName.substr(fileName.size() - 4) != ".hdr") {
		throw std::runtime_error{"invalid fileName.\n"};
	}
	// convert pixels to float data
	constexpr uint32_t channels = 3u;
	const int pixelCnt = resolution.x() * resolution.y();
	std::unique_ptr<float[]> imageData{new float[pixelCnt * channels]};
	for (auto i = 0u; i < pixelCnt; ++i) {
		Vector3f color;
		if (options.ignoreWeight) {
			color = pixels[i].energy;
		}
		else {
			color = getPixelColor(i);
		}
		imageData[channels * i] = color.x();
		imageData[channels * i + 1] = color.y();
		imageData[channels * i + 2] = color.z();
	}

	const auto resCode =
		stbi_write_hdr(fileName.c_str(), resolution.x(), resolution.y(), channels, imageData.get());
	if (resCode == 0) {
		throw std::runtime_error{"save file failed.\n"};
	}
}

void Film::clear()
{
	for (auto& pixel : pixels) {
		pixel.weight = 0.f;
		pixel.energy = {0, 0, 0};
	}
}

void FilmTile::addSample(const ColorRGB& color, const Vector2f& samplePos, float weight)
{
	const auto index = getIndexFromSample(samplePos);
	if (index == -1)
		return;
	auto& pixel = tilePixels[index];
	pixel.energy += color * weight;
	pixel.weight += weight;
}

int FilmTile::getIndexFromSample(const Vector2f& samplePos) const
{
	const int x = (int)samplePos.x() - topLeft.x();
	const int y = (int)samplePos.y() - topLeft.y();
	if (x < 0 || x > tileExtent.x() || y < 0 || y > tileExtent.y())
		return -1;
	const auto index = y * tileExtent.x() + x;
	return index;
}

Vector2i FilmTile::getSampleCoordFromIndex(int index) const
{
	const int row = index / tileExtent.x() + topLeft.x();
	const int col = index % tileExtent.x() + topLeft.y();
	return {col, row};
}
FilmTile::TilePixelIterator::TilePixelIterator(const Vector2i& p, const FilmTile* tile)
	: p(p), tile(tile)
{
}
