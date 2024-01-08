//
// Created by Frank on 2024/1/4.
//

#ifndef XD_RT_NEARESTFILTER_H
#define XD_RT_NEARESTFILTER_H
#include "ImageFilter2D.h"
namespace xd {

class NearestFilter : public ImageFilter2D {
public:
	NearestFilter(WrapMode wrap_s, WrapMode wrap_t);
	ColorRGBA filter(const std::shared_ptr<Image2D>& image, const Vector2f& pos) const override;
};

}  // namespace xd

#endif	// XD_RT_NEARESTFILTER_H
