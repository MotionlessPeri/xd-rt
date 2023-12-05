//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_CORETYPES_H
#define XD_RT_CORETYPES_H
#include "MathType.h"
namespace xd {
class BxDF;
class Lambertian;
class PerfectReflection;

class Camera;
class PerspCamera;
class OrthoCamera;

class Film;

class HitAccel;
class BVHNode;
class NoAccel;
class AABB;

class HitRecord;

class HitSolver;
class NaiveHitSolver;
class BVHHitSolver;
class EmbreeHitSolver;

class Integrator;
class SamplerIntegrator;
class MIDirectIntegrator;

class Light;
class PointLight;
class DomeLight;

class Material;
class MatteMaterial;
class PerfectReflectionMaterial;

class Model;
class Sphere;
class Box;
class TriangleMesh;
class Triangle;

class Primitive;

class Ray;

class Sampler;
class SimpleSampler;

class Scene;
class SceneBuilder;

template <typename ReturnType, typename SampleType>
class Texture;
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
using SphereTexturef = SphereTexture<float>;

template <typename ReturnType>
class UVTexture;

using UVTextureF = UVTexture<float>;
using UVTextureRGB = UVTexture<ColorRGB>;
using UVTextureRGBA = UVTexture<ColorRGBA>;
}  // namespace xd
#endif	// XD_RT_CORETYPES_H
