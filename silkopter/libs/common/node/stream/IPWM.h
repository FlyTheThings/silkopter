#pragma once

#include "IStream.h"

namespace silk
{
namespace node
{
namespace stream
{

class IPWM : public IScalar_Stream<Type::PWM>
{
public:
    typedef float                   Value; //0 .. 1 representing duty cycle
    typedef stream::Sample<Value>     Sample;
    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};
DECLARE_CLASS_PTR(IPWM);


}
}
}