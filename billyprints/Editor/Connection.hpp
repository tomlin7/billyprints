#pragma once

class Connection
{
public:
    /// `id` of input node
    void* InputNode = nullptr;
    /// Descriptor of input slot
    const char* InputSlot = nullptr;

    /// `id` of output node
    void* OutputNode = nullptr;
    /// Descriptor of output slot
    const char* OutputSlot = nullptr;

    bool operator==(const Connection& other) const;
    bool operator!=(const Connection& other) const;
};

