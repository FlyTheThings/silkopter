#include "def_lang/impl/Variant_Type.h"
#include "def_lang/impl/Variant_Value.h"
#include "def_lang/IAttribute.h"
#include "def_lang/IString_Value.h"
#include "def_lang/ILiteral_Attribute.h"
#include "def_lang/Qualified_Type.h"

namespace ts
{

Variant_Type::Variant_Type(std::string const& name)
    : Symbol_EP(name)
    , m_ui_name(name)
{
}

Variant_Type::Variant_Type(Variant_Type const& other, std::string const& name)
    : Symbol_EP(other, name)
    , Attribute_Container_EP(other)
    , m_inner_types(other.m_inner_types)
    , m_ui_name(name)
    , m_native_type(other.m_ui_name)
{
}


Result<void> Variant_Type::init(std::vector<std::shared_ptr<const ITemplate_Argument>> const& arguments)
{
    if (arguments.size() <= 1)
    {
        return Error("Expected more than one template argument, got " + std::to_string(arguments.size()));
    }

    for (std::shared_ptr<const ITemplate_Argument> argument: arguments)
    {
        std::shared_ptr<const Qualified_Type> inner_type = std::dynamic_pointer_cast<const Qualified_Type>(argument);
        if (!inner_type)
        {
            return Error("Invalid template argument. Expected type");
        }
        m_inner_types.push_back(inner_type);
    }

    return success;
}

std::string const& Variant_Type::get_ui_name() const
{
    return m_ui_name;
}
Symbol_Path Variant_Type::get_native_type() const
{
    return m_native_type;
}

Result<void> Variant_Type::validate_attribute(IAttribute const& attribute)
{
    if (attribute.get_name() == "ui_name")
    {
        if (ILiteral_Attribute const* att = dynamic_cast<ILiteral_Attribute const*>(&attribute))
        {
            IString_Value const* value = dynamic_cast<IString_Value const*>(&att->get_value());
            if (!value)
            {
                return Error("Attribute '" + attribute.get_name() + "' has to be a string.");
            }
            m_ui_name = value->get_value();
            return success;
        }
        else
        {
            return Error("Attribute '" + attribute.get_name() + "' has to be a string literal.");
        }
    }
    else if (attribute.get_name() == "native_type")
    {
        if (ILiteral_Attribute const* att = dynamic_cast<ILiteral_Attribute const*>(&attribute))
        {
            IString_Value const* value = dynamic_cast<IString_Value const*>(&att->get_value());
            if (!value)
            {
                return Error("Attribute '" + attribute.get_name() + "' has to be a string.");
            }
            m_native_type = value->get_value();
            return success;
        }
        else
        {
            return Error("Attribute '" + attribute.get_name() + "' has to be a string literal.");
        }
    }
    else
    {
        return Error("Attribute " + attribute.get_name() + " not supported");
    }
}

std::shared_ptr<IType> Variant_Type::clone(std::string const& name) const
{
    return std::make_shared<Variant_Type>(*this, name);
}
std::shared_ptr<IType> Variant_Type::alias(std::string const& name) const
{
    std::shared_ptr<Variant_Type> alias = std::make_shared<Variant_Type>(*this, name);
    alias->m_native_type = Symbol_Path(); //clear the native type as this is an alias
    alias->m_aliased_type = this->shared_from_this();
    return alias;
}
std::shared_ptr<const IType> Variant_Type::get_aliased_type() const
{
    return m_aliased_type;
}

size_t Variant_Type::get_inner_qualified_type_count() const
{
    return m_inner_types.size();
}

std::shared_ptr<const Qualified_Type> Variant_Type::get_inner_qualified_type(size_t idx) const
{
    return m_inner_types[idx];
}

boost::optional<size_t> Variant_Type::find_inner_qualified_type_idx(std::shared_ptr<const IType> type) const
{
    auto it = std::find_if(m_inner_types.begin(), m_inner_types.end(), [&](std::shared_ptr<const Qualified_Type> const& t) { return t->get_type() == type; });
    return it != m_inner_types.end() ? std::distance(m_inner_types.begin(), it) : boost::optional<size_t>();
}

std::shared_ptr<IValue> Variant_Type::create_value() const
{
    return create_specialized_value();
}

std::shared_ptr<Variant_Type::value_type> Variant_Type::create_specialized_value() const
{
    return std::make_shared<Variant_Value>(shared_from_this());
}

}
