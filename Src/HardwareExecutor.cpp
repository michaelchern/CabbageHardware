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

        globalHardwareContext.mainDevice->deviceManager << runCommand << globalHardwareContext.mainDevice->deviceManager.endCommands();

        rasterizerPipelineBegin = false;
    }

    if (computePipelineBegin)
    {
        computePipelineBegin = false;
    }
    return *this;
}