//
// Created by Frank on 2024/1/5.
//
#include "ImageTexture.h"
#include "filter/ImageFilter2D.h"
using namespace xd;

ImageTexture::ImageTexture(std::shared_ptr<Image2D> image,
						   std::shared_ptr<ImageFilter2D> filter,
						   std::shared_ptr<Mapping2D> mapping)
	: image(std::move(image)), filter(std::move(filter)), mapping(std::move(mapping))
{
}

ColorRGBA ImageTexture::sample(const TextureEvalContext& ctx) const
{
	return filter->filter(image, mapping->map(ctx));
}
