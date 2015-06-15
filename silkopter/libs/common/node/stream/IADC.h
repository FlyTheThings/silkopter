#pragma once

#include "Stream_Base.h"

namespace silk
{
namespace node
{
namespace stream
{

class IADC : public IScalar_Stream<Type::ADC, Bias_Scale_Calibration_Data<float>>
{
public:
    typedef float                   Value; //0 .. 1
    typedef stream::Sample<Value>     Sample;
    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};
DECLARE_CLASS_PTR(IADC);


}
}
}
