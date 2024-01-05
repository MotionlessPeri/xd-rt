//
// Created by Frank on 2024/1/5.
//

#ifndef XD_RT_IMAGETEXTURE_H
#define XD_RT_IMAGETEXTURE_H
#include "Image.h"
#include "Texture.h"
namespace xd {
template <typename ReturnType, typename SampleType>
class ImageTexture : public Texture<ReturnType, SampleType> {};

template <typename ReturnType>
class ImageTexture2D : public ImageTexture<ReturnType, Vector2f> {};
}  // namespace xd
#endif	// XD_RT_IMAGETEXTURE_H
