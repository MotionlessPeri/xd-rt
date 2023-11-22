//
// Created by Frank on 2023/9/30.
//

#ifndef XD_RT_SCENELOADER_H
#define XD_RT_SCENELOADER_H
#include "CoreTypes.h"
namespace xd {
struct LoadSceneOptions {};
class SceneLoader {
public:
	virtual std::shared_ptr<Scene> load(const std::string& path,
										const LoadSceneOptions& options) = 0;
};

}  // namespace xd
#endif	// XD_RT_SCENELOADER_H
