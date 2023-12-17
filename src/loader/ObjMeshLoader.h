//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_OBJMESHLOADER_H
#define XD_RT_OBJMESHLOADER_H
#include "MeshLoader.h"
#include "model/ModelTypes.h"
namespace xd {
class ObjMeshLoader : public MeshLoader {
public:
	std::shared_ptr<TriangleMesh> load(const std::string& path,
									   const LoadMeshOptions& options = {}) override;
};
}  // namespace xd
#endif	// XD_RT_OBJMESHLOADER_H
