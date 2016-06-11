#pragma once

namespace silk
{
namespace uav
{
struct IBus_Descriptor;
}
}


namespace silk
{
namespace bus
{

class IBus : q::util::Noncopyable
{
    DEFINE_RTTI_BASE_CLASS(IBus);
public:
    virtual ~IBus() = default;

    virtual bool init(std::shared_ptr<uav::IBus_Descriptor> descriptor) = 0;
    virtual std::shared_ptr<uav::IBus_Descriptor> get_descriptor() const = 0;
};

}
}
