#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include "initialization.h"
#include "utils.h"
#include "task_common.h"
#include "A1task2.h"

void A1_Task2::defaultValues() {
    std::vector<int> inputVec = incArray();
    // Fill buffers
}

void A1_Task2::prepare(unsigned int size_w, unsigned int size_h) {
    this->workloadSize_w = size_w;
    this->workloadSize_h = size_h;
    this->workloadSize = size_h * size_w;

    // ### Fill the descriptorLayoutBindings ###

    // ### Create DescriptorSet ###

    // ### create buffers ###

    // ### Fills Buffer ###
    this->defaultValues();

    // ### Bind buffers to descriptor set ### (calls update several times)

    // ### Create Pipeline Layout + Push Constant ###
}

void A1_Task2::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file) {
    std::string shader_file = get_shader_path(file);

    // ### Create ShaderModule ###
    // Same as in task1

    // ### Create Pipeline ###

    // ### Dispatch Work ###
}

void A1_Task2::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct& pushConstant) {
    // ### You can copy/paste the function of A1_Task1 here ###
}
