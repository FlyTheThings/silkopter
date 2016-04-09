#pragma once

#include "IType.h"

namespace ts
{

class IBool_Value;

class IBool_Type : virtual public IType
{
public:
    typedef IBool_Value value_type;
    typedef bool fundamental_type;

    virtual auto get_specialized_default_value() const -> std::shared_ptr<const value_type> = 0;
    virtual auto create_specialized_value() const -> std::unique_ptr<value_type> = 0;
};

}