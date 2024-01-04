//
// Created by Frank on 2024/1/4.
//

#ifndef XD_RT_TENTFILTER_H
#define XD_RT_TENTFILTER_H
#include "Filter.h"
namespace xd {

class TentFilter : public Filter2D {
public:
	TentFilter(WrapMode mode, std::shared_ptr<Image2D> image);

	ColorRGBA filter(const Vector2f& pos) const override;
};

}  // namespace xd

#endif	// XD_RT_TENTFILTER_H
