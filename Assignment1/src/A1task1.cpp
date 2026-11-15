#include <iostream>
#include <cstdlib>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <vector>
#include <algorithm>
#include "initialization.h"
#include "utils.h"
#include "task_common.h"
#include "A1task1.h"

#include <cstdlib> // for rand
#include <ctime>   // for time

void A1_Task1::prepare(unsigned int size)
{
    this->workloadSize = size;

    // ### create buffers ###
    const vk::DeviceSize byteSize = workloadSize * sizeof(int);

    createBuffer(app.pDevice, app.device, byteSize,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, "inBuffer1",
                 this->inBuffer1.buf, this->inBuffer1.mem);
    createBuffer(app.pDevice, app.device, byteSize,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, "inBuffer2",
                 this->inBuffer2.buf, this->inBuffer2.mem);

    createBuffer(app.pDevice, app.device, byteSize,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eDeviceLocal, "outBuffer",
                 this->outBuffer.buf, this->outBuffer.mem);

    // ### Fills inBuffer1 and inBuffer2 ###
    // This creates default values
    this->defaultValues();
    // ### Fill the descriptorLayoutBindings  ###
    Cmn::addStorage(task.bindings, 0); // inBuffer1
    Cmn::addStorage(task.bindings, 1); // inBuffer2
    Cmn::addStorage(task.bindings, 2); // outBuffer

    // ### Create DescriptorSet ###
    Cmn::createDescriptorSetLayout(app.device, task.bindings, task.descriptorSetLayout);

    // ### DescriptorSet is created but not filled yet ###
    Cmn::createDescriptorPool(app.device, task.bindings, task.descriptorPool, 1);

    // ### Bind buffers to descriptor set ### (calls update several times)
    Cmn::allocateDescriptorSet(app.device, task.descriptorSet, task.descriptorPool, task.descriptorSetLayout);

    Cmn::bindBuffers(app.device, inBuffer1.buf, task.descriptorSet, 0);
    Cmn::bindBuffers(app.device, inBuffer2.buf, task.descriptorSet, 1);
    Cmn::bindBuffers(app.device, outBuffer.buf, task.descriptorSet, 2);

    // ### Push Constant ###
    vk::PushConstantRange pcr(vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushStruct));
    vk::PipelineLayoutCreateInfo pipInfo(vk::PipelineLayoutCreateFlags(), 1U, &task.descriptorSetLayout, 1U, &pcr);
    task.pipelineLayout = app.device.createPipelineLayout(pipInfo);
    // ### Create Pipeline Layout ###
    // task.pipelineLayout = ...

    // ### Preparation work done! ###
}

void A1_Task1::compute(uint32_t dx, uint32_t dy, uint32_t dz, std::string file)
{
    std::string shader_file = get_shader_path(file);

    // ### Create ShaderModule ###
    std::string compute = "./build/shaders/" + file + ".comp.spv";
    app.device.destroyShaderModule(task.cShader);
    Cmn::createShader(app.device, task.cShader, compute);

    std::array<vk::SpecializationMapEntry, 1> specEntries = std::array<vk::SpecializationMapEntry, 1>{{{0U, 0U, sizeof(int)}}};
    std::array<int, 1> specValues = {int(dx)};
    vk::SpecializationInfo specInfo = vk::SpecializationInfo(CAST(specEntries), specEntries.data(), CAST(specValues) * sizeof(int), specValues.data());

    vk::PipelineShaderStageCreateInfo stageInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eCompute, task.cShader, "main", &specInfo);

    // ### Create Pipeline ###
    vk::ComputePipelineCreateInfo computeInfo(vk::PipelineCreateFlags(), stageInfo, task.pipelineLayout);
    app.device.destroyPipeline(task.pipeline);
    task.pipeline = app.device.createComputePipeline(nullptr, computeInfo, nullptr).value;
    // ### Dispatch Work ###
    uint32_t groupCount = (workloadSize + dx - 1) / dx;
    PushStruct push{workloadSize};

    this->dispatchWork(groupCount, 1U, 1U, push);
}

void A1_Task1::dispatchWork(uint32_t dx, uint32_t dy, uint32_t dz, PushStruct &pushConstant)
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
