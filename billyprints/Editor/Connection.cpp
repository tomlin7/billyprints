#include "Connection.hpp"

bool Connection::operator==(const Connection& other) const
{
    return InputNode == other.InputNode &&
        InputSlot == other.InputSlot &&
        OutputNode == other.OutputNode &&
        OutputSlot == other.OutputSlot;
};

bool Connection::operator!=(const Connection& other) const
{
    return !operator ==(other);
}
