#pragma once

#include "Stream_Base.h"

namespace silk
{
namespace node
{
namespace stream
{

class ILinear_Acceleration : public ISpatial_Stream<Type::LINEAR_ACCELERATION, Space::LOCAL>
{
public:
    typedef math::vec3f       Value; //meters per second^2
    typedef stream::Sample<Value>     Sample;
    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};
DECLARE_CLASS_PTR(ILinear_Acceleration);

class IENU_Linear_Acceleration : public ISpatial_Stream<Type::LINEAR_ACCELERATION, Space::ENU>
{
public:
    typedef math::vec3f       Value; //meters per second^2
    typedef stream::Sample<Value>     Sample;
    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};
DECLARE_CLASS_PTR(IENU_Linear_Acceleration);

class IECEF_Linear_Acceleration : public ISpatial_Stream<Type::LINEAR_ACCELERATION, Space::ECEF>
{
public:
    typedef math::vec3f       Value; //meters per second^2
    typedef stream::Sample<Value>     Sample;
    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};
DECLARE_CLASS_PTR(IECEF_Linear_Acceleration);


}
}
}
