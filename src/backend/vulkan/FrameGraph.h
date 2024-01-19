//
// Created by Frank on 2024/1/17.
//

#ifndef XD_RT_FRAMEGRAPH_H
#define XD_RT_FRAMEGRAPH_H
#include <memory>

#include "VulkanTypes.h"
namespace xd {
class FrameGraphPass {
private:
	std::shared_ptr<VulkanSubpass> subpass = nullptr;
	std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
};
class FrameGraphBuilder {
public:
private:
};
class FrameGraph {};

}  // namespace xd

#endif	// XD_RT_FRAMEGRAPH_H
