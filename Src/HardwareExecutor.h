#pragma once

struct HardwareExecutor
{
    HardwareExecutor();
    ~HardwareExecutor();

    HardwareExecutor &operator<<(const HardwareExecutor &)
    {
    }

    HardwareExecutor &commit()
    {
    }
};