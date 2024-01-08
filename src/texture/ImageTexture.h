//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_IMAGETEXTURE_H
#define XD_RT_IMAGETEXTURE_H
#include "CoreTypes.h"
#include "Mapping.h"
#include "Texture.h"
#include "filter/FilterTypes.h"
#include "filter/ImageFilter2D.h"

namespace xd {
class ImageTexture : public Texture {
public:
	ImageTexture(std::shared_ptr<Image2D> image,
				 std::shared_ptr<ImageFilter2D> filter,
				 std::shared_ptr<Mapping2D> mapping);

	ColorRGBA sample(const TextureEvalContext& ctx) const override;
	[[nodiscard]] std::shared_ptr<Image2D> getImage() const { return image; }
	[[nodiscard]] std::shared_ptr<ImageFilter2D> getFilter() const { return filter; }
	[[nodiscard]] std::shared_ptr<Mapping2D> getMapping() const { return mapping; }

private:
	std::shared_ptr<Image2D> image;
	std::shared_ptr<ImageFilter2D> filter;
	std::shared_ptr<Mapping2D> mapping;
};
}  // namespace xd
#endif	// XD_RT_IMAGETEXTURE_H
