#pragma once

#include "IValue.h"

namespace ts
{

class IString_Type;

class IString_Value : virtual public IValue
{
public:
    typedef IString_Type type_type;
    typedef std::string fundamental_type;

    virtual auto get_specialized_type() const -> std::shared_ptr<type_type> = 0;

    virtual auto set_value(fundamental_type value) -> Result<void> = 0;
    virtual auto get_value() const -> fundamental_type = 0;
};

}