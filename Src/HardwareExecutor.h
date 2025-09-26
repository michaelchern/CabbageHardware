#pragma once

struct HardwareExecutor
{
    HardwareExecutor() = default;
    ~HardwareExecutor() = default;

    HardwareExecutor &operator<<(const HardwareExecutor &)
    {
        return *this;
    }

    HardwareExecutor &commit()
    {
        return *this;
    }
};