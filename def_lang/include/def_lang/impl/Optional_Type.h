#pragma once

#include "def_lang/IOptional_Type.h"
#include "def_lang/ep/Attribute_Container_EP.h"
#include "def_lang/ep/Symbol_EP.h"

namespace ts
{

class Optional_Type final: virtual public IOptional_Type, public Symbol_EP, public Attribute_Container_EP, public std::enable_shared_from_this<Optional_Type>
{
public:
    typedef IOptional_Value value_type;

    Optional_Type(std::string const& name);
    Optional_Type(Optional_Type const& other, std::string const& name);

    Result<void> init(std::vector<std::shared_ptr<const ITemplate_Argument>> const& arguments) override;

    std::shared_ptr<IType> clone(std::string const& name) const override;
    std::shared_ptr<IType> alias(std::string const& name) const override;
    std::shared_ptr<const IType> get_aliased_type() const override;

    std::string const& get_ui_name() const override;
    Symbol_Path get_native_type() const override;

    std::shared_ptr<IValue> create_value() const override;
    std::shared_ptr<IOptional_Value> create_specialized_value() const override;

    std::shared_ptr<const Qualified_Type> get_inner_qualified_type() const override;

protected:
    Result<void> validate_attribute(IAttribute const& attribute) override;

private:
    std::shared_ptr<const Qualified_Type> m_inner_qualified_type;
    std::shared_ptr<const IType> m_aliased_type;
    std::string m_ui_name;
    Symbol_Path m_native_type;
};

}
