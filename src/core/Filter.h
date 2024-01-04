//
// Created by Frank on 2024/1/3.
//

#ifndef XD_RT_FILTER_H
#define XD_RT_FILTER_H
#include "Image.h"
namespace xd {
enum class WrapMode { CLAMP, REPEAT };
// Note: for now Filter2D only applies to Image2D
// We can loose this limitation if needed.
class Filter2D {
public:
	explicit Filter2D(std::shared_ptr<Image2D> image) : image(std::move(image)) {}

	Filter2D(WrapMode mode, std::shared_ptr<Image2D> image) : mode(mode), image(std::move(image)) {}

	virtual ~Filter2D() = default;
	/**
	 * \brief calculate the filtered result respected to pos
	 * \param pos the position of the center of the sample. Pos lies in [0, width] x [0, height]
	 * \return the filtered sample value
	 */
	virtual ColorRGBA filter(const Vector2f& pos) const = 0;

protected:
	/**
	 * \brief produce image pixel coords respect to WrapMode
	 * \param row the row coords of image pixel
	 * \param col the col coords of image pixel
	 * \return the produced image pixel coords respect to WrapMode
	 */
	auto getPixelCoords(int row, int col) const
	{
		struct Ret {
			uint32_t row, col;
		};
		switch (mode) {
			case WrapMode::CLAMP: {
				col = std::clamp<int>(col, 0, image->width - 1);
				row = std::clamp<int>(row, 0, image->height - 1);
				break;
			}
			case WrapMode::REPEAT: {
				col %= image->width;
				if (col < 0)
					col += image->width;
				row %= image->height;
				;
				if (row < 0)
					row += image->height;
				break;
			}
			default:
				break;
		}
		return Ret{static_cast<uint32_t>(row), static_cast<uint32_t>(col)};
	}
	ColorRGBA getPixelValue(uint32_t row, uint32_t col) const
	{
		const auto intCoords = getPixelCoords(row, col);
		return image->getPixelValue(intCoords.row, intCoords.col);
	}
	WrapMode mode = WrapMode::CLAMP;
	std::shared_ptr<Image2D> image;
};

}  // namespace xd
#endif	// XD_RT_FILTER_H
