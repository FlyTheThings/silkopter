#pragma once

#include <memory>
#include "impl/Symbol_Impl.h"

namespace ts
{

class IType;
class IValue;

class Member_Def final : public Symbol_Impl
{
public:

    Member_Def(std::string const& name, std::shared_ptr<IType> type, std::unique_ptr<IValue> default_value);
    ~Member_Def();

    auto get_type() const -> std::shared_ptr<IType>;
    auto get_default_value() const -> std::shared_ptr<const IValue>;

private:
    std::shared_ptr<IType> m_type;
    std::shared_ptr<IValue> m_default_value;
};

}
