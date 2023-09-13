//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_CORETYPES_H
#define XD_RT_CORETYPES_H
#include "MathType.h"
namespace xd {
class HitRecord;

class Camera;
class PerspCamera;
class OrthoCamera;

class Ray;

class Model;
class TriangleMesh;
class Triangle;

class HitAccel;
class BVHNode;
class NoAccel;

class Primitive;

class AABB;

class Material;
class MatteMaterial;

class Light;
class PointLight;

class Scene;
class HitSolver;

class BRDF;
class Lambertian;
template <typename ReturnType, typename SampleType>
class Texture;
template <typename ReturnType>
using Texture2D = Texture<ReturnType, Vector2f>;

template <typename SampleType>
using TextureF = Texture<float, SampleType>;

template <typename SampleType>
using TextureColor = Texture<ColorRGB, SampleType>;

using Texture2DF = Texture<float, Vector2f>;
using Texture2DC = Texture<ColorRGB, Vector2f>;

template <typename ReturnType>
class ConstantTexture;

using ConstantTextureF = ConstantTexture<float>;
using ConstantTextureColor = ConstantTexture<ColorRGB>;
}  // namespace xd
#endif	// XD_RT_CORETYPES_H
