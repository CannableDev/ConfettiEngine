#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include <vector>
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Application;

class GraphicsPipeline {
public:
	GraphicsPipeline(VkInstance& i, VkDevice& d, VkExtent2D& e, std::string v, std::string f);
	// GraphicsPipeline(VkInstance& i, VkDevice& d, std::string v, std::string g, std::string f);
	~GraphicsPipeline();

	GraphicsPipeline& operator=(const GraphicsPipeline& other);

	const VkPipelineLayout& getLayout();

private:
	friend class Application;

	VkShaderModule createModule(const std::vector<char>& code);

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkPipelineLayout pipelineLayout;

	VkExtent2D& extents;
	VkInstance& instance;
	VkDevice& device;
};

#endif