#include"CabbageDisplayer.h"

#include <Hardware/GlobalContext.h>



HardwareExecutor& HardwareExecutor::operator()(ExecutorType type)
{
    this->type = type;
    return *this;
}


HardwareExecutor& HardwareExecutor::commit()
{
    if (rasterizerPipelineBegin)
    {
        auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
            vkCmdEndRenderPass(commandBuffer);
        };

        globalHardwareContext.mainDevice->deviceManager << runCommand;
    }

    if (computePipelineBegin)
    {
    }

    if (rasterizerPipelineBegin || computePipelineBegin)
    {
        globalHardwareContext.mainDevice->deviceManager << globalHardwareContext.mainDevice->deviceManager.endCommands();

        computePipelineBegin = false;
        rasterizerPipelineBegin = false;
    }

    return *this;
}