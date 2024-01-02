//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_TEXTURETYPES_H
#define XD_RT_TEXTURETYPES_H
#include "CoreTypes.h"
#include "MathTypes.h"
namespace xd {
template <typename ReturnType>
using Texture2D = Texture<ReturnType, Vector2f>;

template <typename ReturnType>
using Texture3D = Texture<ReturnType, Vector3f>;

template <typename SampleType>
using TextureF = Texture<float, SampleType>;

template <typename SampleType>
using TextureRGB = Texture<ColorRGB, SampleType>;

using Texture2DF = Texture<float, Vector2f>;
using Texture2DRGB = Texture<ColorRGB, Vector2f>;
using Texture2DRGBA = Texture<ColorRGBA, Vector2f>;

template <typename ReturnType, typename SampleType>
class ConstantTexture;

using ConstantTexture2DF = ConstantTexture<float, Vector2f>;
using ConstantTexture2DRGB = ConstantTexture<ColorRGB, Vector2f>;
using ConstantTexture2DRGBA = ConstantTexture<ColorRGBA, Vector2f>;
template <typename ReturnType>
class SphereTexture;

using SphereTextureRGB = SphereTexture<ColorRGB>;
using SphereTextureF = SphereTexture<float>;

template <typename ReturnType>
class UVTexture;

using UVTextureF = UVTexture<float>;
using UVTextureRGB = UVTexture<ColorRGB>;
using UVTextureRGBA = UVTexture<ColorRGBA>;
}  // namespace xd
#endif	// XD_RT_TEXTURETYPES_H
