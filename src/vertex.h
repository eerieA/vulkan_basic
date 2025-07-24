#pragma once

#include <vulkan/vulkan.h>

namespace veng {
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription description = {};
			description.binding = 0;
			description.stride = sizeof(Vertex);
			description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return description;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> description = {};

			description[0].binding = 0;
			description[0].location = 0;
			description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			description[0].offset = offsetof(Vertex, position);

			description[1].binding = 0;
			description[1].location = 1;
			description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			description[1].offset = offsetof(Vertex, color);

			return description;
		}
	};
}