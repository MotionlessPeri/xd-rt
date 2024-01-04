//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_CORETYPES_H
#define XD_RT_CORETYPES_H
#include "MathTypes.h"
namespace xd {
class AABB;

class BxDF;

class Camera;

class Film;

class Filter;

class HitAccel;

struct HitRecord;
struct ShadingDerivatives;
struct LocalGeomParams;

class HitSolver;

class Image;

class Integrator;

class Light;

class LightSampler;

class Material;
class PhysicalPlausibleMaterial;

class Model;

class Primitive;

class Ray;

class Sampler;

class Scene;
class SceneBuilder;

template <typename ReturnType, typename SampleType>
class Texture;

}  // namespace xd
#endif	// XD_RT_CORETYPES_H
