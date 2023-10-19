//
// Created by Frank on 2023/10/1.
//

#ifndef XD_RT_MATERIALLOADER_H
#define XD_RT_MATERIALLOADER_H
#include "CoreTypes.h"
namespace xd {
struct LoadMaterialOptions {};
class MaterialLoader {
public:
	virtual std::shared_ptr<Material> load(const std::string& path,
										   const LoadMaterialOptions& options) = 0;
};

}  // namespace xd
#endif	// XD_RT_MATERIALLOADER_H
