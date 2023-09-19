//
// Created by Frank on 2023/9/19.
//

#ifndef XD_RT_TEXTUREFACTORY_H
#define XD_RT_TEXTUREFACTORY_H
#include "Texture.h"
namespace xd {
class TextureFactory {
public:
	static std::shared_ptr<SphereTexture<Vector3f>> loadSphereTexture3f(const std::string& path);
};
}  // namespace xd
#endif	// XD_RT_TEXTUREFACTORY_H
