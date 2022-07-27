#include "Connection.hpp"

namespace Billyprints {
    bool Connection::operator==(const Connection& other) const
    {
        return inputNode == other.inputNode &&
            inputSlot == other.inputSlot &&
            outputNode == other.outputNode &&
            outputSlot == other.outputSlot;
    };

    bool Connection::operator!=(const Connection& other) const
    {
        return !operator ==(other);
    }
}