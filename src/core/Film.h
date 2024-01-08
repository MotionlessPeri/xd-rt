//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_FILM_H
#define XD_RT_FILM_H

#include <MathUtil.h>
#include <cassert>
#include <mutex>
#include <utility>
#include <vector>
#include "MathTypes.h"
namespace xd {
struct FilmPixel {
	ColorRGB energy{0, 0, 0};
	float weight = 0.f;

	friend bool operator==(const FilmPixel& lhs, const FilmPixel& rhs)
	{
		return lhs.energy.isApprox(rhs.energy) && fuzzyEqual(lhs.weight, rhs.weight);
	}

	friend bool operator!=(const FilmPixel& lhs, const FilmPixel& rhs) { return !(lhs == rhs); }
};
struct FilmTile {
public:
	FilmTile(const Vector2i& topLeft, const Vector2i& bottomRight);
	/**
	 * add a sample to the tile.
	 * The samplePos lies in [0, width] x [0, height] considering the whole film plane.
	 * If the samplePos is out of bound, it'll be neglected.
	 * @param color the color of the sample
	 * @param samplePos the position of the sample. The samplePos lies in [0, width] x [0, height].
	 * Note that the left and top boarder is considered inside the pixel, while right and bottom
	 * boarder not
	 * @param weight the weight of the sample
	 */
	void addSample(const ColorRGB& color, const Vector2f& samplePos, float weight = 1.f);
	/**
	 * get the index of pixel in tilePixels respect to samplePos. If the samplePos is out of bound,
	 * -1 will be returned
	 * @param samplePos the position of the sample. See FilmTile::addSample()
	 * @return the index of pixel in tilePixels
	 */
	int getIndexFromSample(const Vector2f& samplePos) const;
	/**
	 * calculate sample coords according to given index
	 * @param index index in tilePixels
	 * @return a two dimensional integer coord represents pixel position in the whole film
	 */
	Vector2i getSampleCoordFromIndex(int index) const;

	class TilePixelIterator {
	public:
		TilePixelIterator(Vector2i p, const FilmTile* tile);
		TilePixelIterator operator++()
		{
			p.x()++;
			if (p.x() > tile->bottomRight.x()) {
				p.y()++;
				p.x() = tile->topLeft.x();
			}
			return *this;
		}
		const TilePixelIterator operator++(int)
		{
			auto old = *this;
			this->operator++();
			return old;
		}
		bool operator==(const TilePixelIterator& rhs) const
		{
			return p == rhs.p && tile == rhs.tile;
		}
		Vector2i operator*() const { return p; }

	protected:
		Vector2i p;
		const FilmTile* tile;
	};
	TilePixelIterator begin() { return {topLeft, this}; }
	TilePixelIterator end() { return {{topLeft.x(), bottomRight.y() + 1}, this}; }
	Vector2i operator[](std::size_t i) const
	{
		assert(i >= 0 && i < tileExtent.x() * tileExtent.y());
		return getSampleCoordFromIndex(i);
	}
	std::size_t size() const { return tileExtent.x() * tileExtent.y(); }
	Vector2i getExtent() const { return bottomRight - topLeft; };
	Vector2i topLeft;
	Vector2i bottomRight;
	Vector2i tileExtent;
	std::vector<FilmPixel> tilePixels;
};
class Film {
public:
	Film() = default;
	Film(Vector3f center,
		 const Vector3f& right,
		 const Vector3f& up,
		 uint32_t width,
		 uint32_t height);
	Film(Vector3f center,
		 const Vector3f& right,
		 const Vector3f& up,
		 uint32_t width,
		 uint32_t height,
		 const Vector2i& croppedTopLeft,
		 const Vector2i& croppedBottomRight);
	Film(const Film& other);
	Film(Film&& other) noexcept;
	Film& operator=(const Film& other);
	Film& operator=(Film&& other) noexcept;

	class FilmPixelIterator {
	public:
		FilmPixelIterator(const Vector2i& p, const Film* film);
		FilmPixelIterator operator++()
		{
			p.x()++;
			if (p.x() >= film->pixelExtent.x()) {
				p.y()++;
				p.x() = 0;
			}
			return *this;
		}
		const FilmPixelIterator operator++(int)
		{
			auto old = *this;
			this->operator++();
			return old;
		}
		bool operator==(const FilmPixelIterator& rhs) const
		{
			return p == rhs.p && film == rhs.film;
		}
		const FilmPixel& operator*() const
		{
			const auto index = film->resolution.x() * p.y() + p.x();
			return film->pixels[index];
		}
		FilmPixel& operator*()
		{
			const auto index = film->resolution.x() * p.y() + p.x();
			return film->pixels[index];
		}

	protected:
		Vector2i p;
		Film* film;
	};
	std::unique_ptr<FilmTile> getTile(const Vector2i& topLeft, const Vector2i& bottomRight) const;

	void mergeTileToFilm(std::unique_ptr<FilmTile> tile);
	/**
	 * get color of a given point
	 * @param pos the position of the point. The position is normalized(in [0,1]x[0,1]), taking the
	 * top-left corner as origin, right as x and down as y
	 * @return the color of the point
	 */
	ColorRGB getPixelColor(const Vector2f& pos);

	ColorRGB getPixelColor(int index);

	/**
	 * get world coordinate respect to sample.
	 * @param sample 2d sample that represents a point on the film plane.
	 * The sample's frame is defined as:
	 * Origin: top left corner
	 * X-axis: horizontal edge of the film
	 * X-axis unit: pixel width
	 * Y-axis: vertical edge of the film
	 * Y-axis unit: pixel height
	 * The sample's scope is in [0, resolution.x + filterRadius.x] x [0, resolution.y +
	 * filterRadius.y]
	 * @return the world space position of the sample
	 */
	Vector3f getWorldCoordsFromSample(const Vector2f& sample) const;

	Vector3f getCenter() const { return center; }

	Vector3f getUp() const { return up; }

	Vector3f getRight() const { return right; }

	Vector3f getTowards() const { return towards; }

	Vector2i getResolution() const { return resolution; }

	struct SaveOptions {
		SaveOptions() : ignoreWeight(false) {}
		SaveOptions(bool ignoreWeight) : ignoreWeight(ignoreWeight) {}
		bool ignoreWeight;
	};
	/**
	 * save the content of a film to a .hdr file
	 * @param fileName the path where the hdr file stores. must be end with .hdr
	 */
	void saveToFile(const std::string& fileName,
					const Film::SaveOptions& options = Film::SaveOptions());

	void clear();

	FilmPixel& operator()(uint32_t index) { return pixels[index]; }
	const FilmPixel& operator()(uint32_t index) const { return pixels[index]; }
	FilmPixel& operator()(uint32_t row, uint32_t col)
	{
		return pixels[getIndexFromRowCol(row, col)];
	}
	const FilmPixel& operator()(uint32_t row, uint32_t col) const
	{
		return pixels[getIndexFromRowCol(row, col)];
	}

protected:
	Vector3f getTopLeft() const;

	uint32_t getIndexFromSample(const Vector2f& sample) const;

	uint32_t getIndexFromRowCol(uint32_t row, uint32_t col) const
	{
		return row * resolution.x() + col;
	}
	Vector3f center;
	Vector3f right;
	Vector3f normedRight;
	Vector3f up;
	Vector3f normedUp;
	Vector3f towards;
	Vector2i resolution;
	Vector2f pixelExtent;  // pixel's extent in world frame
	Vector2i croppedTopLeft;
	Vector2i croppedBottomRight;
	std::mutex imageWriteLock;
	std::vector<FilmPixel> pixels;
};

}  // namespace xd

#endif	// XD_RT_FILM_H
