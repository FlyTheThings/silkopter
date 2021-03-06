#pragma once

#include <boost/signals2.hpp>
#include <memory>
#include "Result.h"
#include "IInitializer_List.h"
#include "impl/Initializer_List.h"
#include "Serialization.h"

namespace ts
{

class IType;
class Value_Selector;

class IValue
{
public:
    IValue() = default;
    IValue(IValue const&) = delete;
    IValue& operator=(IValue const&) = delete;

    virtual ~IValue() = default;

    virtual bool is_constructed() const = 0;

    virtual Result<void> construct(IInitializer_List const& initializer_list = Initializer_List({})) = 0;
    virtual Result<void> copy_construct(IValue const& other) = 0;

    virtual Result<void> copy_assign(IValue const& other) = 0;
    virtual Result<void> copy_assign(IInitializer_List const& initializer_list) = 0;

    virtual Result<bool> is_equal(IValue const& other) const = 0;

    virtual std::shared_ptr<IType const> get_type() const = 0;

    virtual Result<void> parse_from_ui_string(std::string const& str) = 0;
    virtual Result<std::string> get_ui_string() const = 0;

    virtual std::shared_ptr<const IValue> select(Value_Selector&& selector) const = 0;
    virtual std::shared_ptr<IValue> select(Value_Selector&& selector) = 0;

    virtual Result<sz::Value> serialize() const = 0;
    virtual Result<void> deserialize(sz::Value const&) = 0;

    template<typename T>
    std::shared_ptr<const T> select_specialized(Value_Selector&& selector) const;

    template<typename T>
    std::shared_ptr<T> select_specialized(Value_Selector&& selector);

    boost::signals2::signal<void()> sig_value_changed;
};

}

#include "impl/IValue.inl"
