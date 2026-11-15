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

void A1_Task2::defaultValues()
{
    std::vector<int> inputVec = incArray();
    // Fill buffers
    fillDeviceWithStagingBuffer(app.pDevice, app.device, app.computeCommandPool, app.computeQueue, inBuffer, inputVec);
}

void A1_Task2::prepare(unsigned int size_w, unsigned int size_h)
{
    this->workloadSize_w = size_w;
    this->workloadSize_h = size_h;
    this->workloadSize = size_h * size_w;
    const vk::DeviceSize byteSize = workloadSize * sizeof(int);

    // ### Fill the descriptorLayoutBindings ###
    Cmn::addStorage(task.bindings, 0); // inBuffer
    Cmn::addStorage(task.bindings, 1); // outBuffer
    // ### Create DescriptorSet ###
    Cmn::createDescriptorSetLayout(app.device, task.bindings, task.descriptorSetLayout);
    Cmn::createDescriptorPool(app.device, task.bindings, task.descriptorPool, 1);

    // ### create buffers ###
    createBuffer(app.pDevice, app.device, byteSize,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, "inBuffer",
                 this->inBuffer.buf, this->inBuffer.mem);
    createBuffer(app.pDevice, app.device, byteSize,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, "outBuffer",
                 this->outBuffer.buf, this->outBuffer.mem);

    // ### Fills Buffer ###
    this->defaultValues();

    // ### Bind buffers to descriptor set ### (calls update several times)
    Cmn::allocateDescriptorSet(app.device, task.descriptorSet, task.descriptorPool, task.descriptorSetLayout);
    Cmn::bindBuffers(app.device, inBuffer.buf, task.descriptorSet, 0);
    Cmn::bindBuffers(app.device, outBuffer.buf, task.descriptorSet, 1);

    // ### Create Pipeline Layout + Push Constant ###
    vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushStruct));
    vk::PipelineLayoutCreateInfo pipInfo(vk::PipelineLayoutCreateFlags(), 1U, &task.descriptorSetLayout, 1U, &pcr);
    task.pipelineLayout = app.device.createPipelineLayout(pipInfo);
}

void A1_Task2::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file)
{
    std::string shader_file = get_shader_path(file);

    // ### Create ShaderModule ###
    // Same as in task1
    std::string compute = "./build/shaders/" + file + ".comp.spv";
    app.device.destroyShaderModule(task.cShader);
    Cmn::createShader(app.device, task.cShader, compute);

    std::array<vk::SpecializationMapEntry, 2> specEntries = std::array<vk::SpecializationMapEntry, 2>{vk::SpecializationMapEntry{0U, 0U, sizeof(int)}, vk::SpecializationMapEntry{1U, sizeof(int), sizeof(int)}};
    std::array<int, 2> specValues = {int(dx), int(dy)};
    vk::SpecializationInfo specInfo = vk::SpecializationInfo(CAST(specEntries), specEntries.data(), CAST(specValues) * sizeof(int), specValues.data());

    vk::PipelineShaderStageCreateInfo stageInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, task.cShader, "main", &specInfo);

    // ### Create Pipeline ###
    vk::ComputePipelineCreateInfo computeInfo(vk::PipelineCreateFlags(), stageInfo, task.pipelineLayout);
    app.device.destroyPipeline(task.pipeline);
    task.pipeline = app.device.createComputePipeline(nullptr, computeInfo, nullptr).value;

    // ### Dispatch Work ###
    uint32_t n_xgroups = (workloadSize_w + dx - 1) / dx;
    uint32_t n_ygroups = (workloadSize_h + dy - 1) / dy;

    PushStruct push{workloadSize_w, workloadSize_h};

    this->dispatchWork(n_xgroups, n_ygroups, 1U, push);
}

void A1_Task2::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct &pushConstant)
{
    // ### Create Command Buffer ###
    vk::CommandBufferAllocateInfo allocInfo(app.computeCommandPool, vk::CommandBufferLevel::ePrimary, 1U);
    vk::CommandBuffer cb = app.device.allocateCommandBuffers(allocInfo)[0];
    // ### Call Begin and register commands ###
    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cb.begin(beginInfo);

    cb.resetQueryPool(app.queryPool, 0, 2);
    cb.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, app.queryPool, 0);
    cb.pushConstants(task.pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushStruct), &pushConstant);
    cb.bindPipeline(vk::PipelineBindPoint::eCompute, task.pipeline);
    cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, task.pipelineLayout, 0U, 1U, &task.descriptorSet, 0U, nullptr);
    cb.dispatch(dx, dy, dz);
    cb.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, app.queryPool, 1);
    // ### End of Command Buffer, enqueue it and use a Fence ###
    cb.end();

    vk::SubmitInfo submitInfo = vk::SubmitInfo(0, nullptr, nullptr, 1, &cb);
    vk::Fence fence = app.device.createFence(vk::FenceCreateInfo());
    app.computeQueue.submit({submitInfo}, fence);
    vk::Result haveIWaited = app.device.waitForFences({fence}, true, uint64_t(-1));
    assert(haveIWaited == vk::Result::eSuccess);
    app.device.destroyFence(fence);

    // ### Collect data from the query Pool ###
    uint64_t timestamps[2];
    vk::Result result = app.device.getQueryPoolResults(app.queryPool, 0, 2, sizeof(timestamps), &timestamps,
                                                       sizeof(timestamps[0]), vk::QueryResultFlagBits::e64);
    assert(result == vk::Result::eSuccess);
    uint64_t timediff = timestamps[1] - timestamps[0];
    vk::PhysicalDeviceProperties properties = app.pDevice.getProperties();
    uint64_t nanoseconds = properties.limits.timestampPeriod * timediff;

    mstime = nanoseconds / 1000000.f;

    app.device.freeCommandBuffers(app.computeCommandPool, 1U, &cb);
}
