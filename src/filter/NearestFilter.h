//
// Created by Frank on 2024/1/4.
//

#ifndef XD_RT_NEARESTFILTER_H
#define XD_RT_NEARESTFILTER_H
#include "Filter.h"
namespace xd {

class NearestFilter : public Filter2D {
public:
	NearestFilter(WrapMode mode, std::shared_ptr<Image2D> image);

	ColorRGBA filter(const Vector2f& pos) const override;
};

}  // namespace xd

#endif	// XD_RT_NEARESTFILTER_H
