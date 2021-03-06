#pragma once

#include <vector>
#include "def_lang/IMember_Def_Container.h"

namespace ts
{

class Member_Def_Container_EP : virtual public IMember_Def_Container
{
public:

    ~Member_Def_Container_EP();

    size_t get_member_def_count() const override;

    Result<void> add_member_def(std::shared_ptr<const IMember_Def> member_def) override;

    std::shared_ptr<IMember_Def const> get_member_def(size_t idx) const override;
    boost::optional<size_t> find_member_def_idx_by_name(std::string const& name) const override;
    std::shared_ptr<IMember_Def const> find_member_def_by_name(std::string const& name) const override;

private:
    std::vector<std::shared_ptr<const IMember_Def>> m_member_defs;
};

}
