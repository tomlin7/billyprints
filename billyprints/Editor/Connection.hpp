#pragma once

namespace Billyprints {
    class Connection
    {
    public:
        /// `id` of input node
        void* inputNode = nullptr;
        /// Descriptor of input slot
        const char* inputSlot = nullptr;

        /// `id` of output node
        void* outputNode = nullptr;
        /// Descriptor of output slot
        const char* outputSlot = nullptr;

        bool operator==(const Connection& other) const;
        bool operator!=(const Connection& other) const;
    };
}
