//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_IMAGEFILTER2D_H
#define XD_RT_IMAGEFILTER2D_H
#include "Filter.h"
#include "Image.h"
namespace xd {
/**
 * \brief filter that focus on a 2d image. The input pos is floating-type point lies in [0, width] x
 * [0, height]
 */
class ImageFilter2D {
public:
	ImageFilter2D(const ImageFilter2D& other) = default;
	ImageFilter2D(ImageFilter2D&& other) noexcept = default;
	ImageFilter2D& operator=(const ImageFilter2D& other) = default;
	ImageFilter2D& operator=(ImageFilter2D&& other) noexcept = default;
	virtual ~ImageFilter2D() = default;

	ImageFilter2D(WrapMode wrap_s, WrapMode wrap_t) : wrapS(wrap_s), wrapT(wrap_t) {}

	virtual ColorRGBA filter(const std::shared_ptr<Image2D>& image, const Vector2f& pos) const = 0;

protected:
	/**
	 * \brief produce image pixel coords respect to WrapMode
	 * \param row the row coords of image pixel
	 * \param col the col coords of image pixel
	 * \return the produced image pixel coords respect to WrapMode
	 */
	auto getPixelCoords(const std::shared_ptr<Image2D>& image, int row, int col) const
	{
		struct Ret {
			uint32_t row, col;
		};
		switch (wrapS) {
			case WrapMode::CLAMP: {
				col = std::clamp<int>(col, 0, image->getWidth() - 1);
				break;
			}
			case WrapMode::REPEAT: {
				col %= image->getWidth();
				if (col < 0)
					col += image->getWidth();
				break;
			}
			default:
				break;
		}
		switch (wrapT) {
			case WrapMode::CLAMP: {
				row = std::clamp<int>(row, 0, image->getHeight() - 1);
				break;
			}
			case WrapMode::REPEAT: {
				row %= image->getHeight();
				if (row < 0)
					row += image->getHeight();
				break;
			}
			default:
				break;
		}
		return Ret{static_cast<uint32_t>(row), static_cast<uint32_t>(col)};
	}

	/**
	 * \brief get pixel value of given coords. Out-of-bounds row and col values(like negative ones)
	 * are allowed and will be handled by getPixelCoords method
	 * \param row row of the pixel
	 * \param col col of the pixel
	 * \return the pixel's value
	 */
	ColorRGBA getPixelValue(const std::shared_ptr<Image2D>& image, int row, int col) const
	{
		const auto intCoords = getPixelCoords(image, row, col);
		return image->getPixelValue(intCoords.row, intCoords.col);
	}
	WrapMode wrapS = WrapMode::CLAMP;
	WrapMode wrapT = WrapMode::CLAMP;
};

}  // namespace xd
#endif	// XD_RT_IMAGEFILTER2D_H
