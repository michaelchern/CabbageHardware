#include"CabbageDisplayer.h"


#include <Hardware/GlobalContext.h>


HardwareExecutor& HardwareExecutor::commit()
{
    if (rasterizerPipelineBegin)
    {
        auto runCommand = [&](const VkCommandBuffer &commandBuffer) {
            vkCmdEndRenderPass(commandBuffer);
        };

        globalHardwareContext.mainDevice->deviceManager << runCommand << globalHardwareContext.mainDevice->deviceManager.endCommands();
    }

    if (computePipelineBegin || rasterizerPipelineBegin)
    {
    }
    return *this;
}