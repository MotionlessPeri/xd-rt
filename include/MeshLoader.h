//
// Created by Frank on 2023/9/8.
//

#ifndef XD_RT_MESHLOADER_H
#define XD_RT_MESHLOADER_H
#include <string>
#include "CoreTypes.h"
#include "Enums.h"
namespace xd {
struct LoadMeshOptions {
	HitAccelMethod method;
};
class MeshLoader {
public:
	virtual ~MeshLoader() = default;
	virtual std::shared_ptr<TriangleMesh> load(const std::string& path,
											   const LoadMeshOptions& options = {}) = 0;

protected:
};

class ObjLoader : public MeshLoader {
public:
	std::shared_ptr<TriangleMesh> load(const std::string& path,
									   const LoadMeshOptions& options = {}) override;
};
}  // namespace xd
#endif	// XD_RT_MESHLOADER_H
