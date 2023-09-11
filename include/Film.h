//
// Created by Frank on 2023/8/16.
//

#ifndef XD_RT_FILM_H
#define XD_RT_FILM_H

#include <utility>
#include <vector>
#include "MathType.h"

namespace xd {
class Film {
public:
	Film(const Vector3f& center,
		 const Vector3f& right,
		 const Vector3f& up,
		 uint32_t width,
		 uint32_t height);

	/**
	 * add color to a pixel
	 * @param color the color to be added
	 * @param samplePos the position of sample. The position is normalized(in [0,1]x[0,1]), taking
	 * the top-left corner as origin, right as x and down as y
	 */
	void addSample(const ColorRGB& color, const Vector2f& samplePos);

	/**
	 * get color of a given point
	 * @param pos the position of the point. The position is normalized(in [0,1]x[0,1]), taking the
	 * top-left corner as origin, right as x and down as y
	 * @return the color of the point
	 */
	ColorRGB getPixelColor(const Vector2f& pos);

	Vector3f getWorldCoordsFromSample(const Vector2f& sample) const;

	Vector3f getCenter() const { return center; }

	Vector3f getUp() const { return up; }

	Vector3f getRight() const { return right; }

	Vector3f getTowards() const { return towards; }

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	/**
	 * save the content of a film to a .hdr file
	 * @param fileName the path where the hdr file stores. must be end with .hdr
	 */
	void saveToFile(const std::string& fileName);

protected:
	Vector3f getTopLeft() const;

	std::pair<Vector3f, Vector3f> getDxDy() const;

	uint32_t getIndexFromSample(const Vector2f& sample) const;

	Vector3f center;
	Vector3f right;
	Vector3f up;
	Vector3f towards;
	uint32_t width;
	uint32_t height;
	inline static constexpr unsigned int channels = 3u;
	std::vector<float> image;  // Note: we use vector<float> here instead of a Pixel struct for
							   // better performance in saveToFile
};

}  // namespace xd

#endif	// XD_RT_FILM_H
