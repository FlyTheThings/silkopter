#pragma once

#include "IStream.h"

namespace silk
{
namespace node
{
namespace stream
{

class IVoltage : public IStream
{
public:
    typedef float             Value; //volts
    typedef Sample<Value>     Sample;

    virtual ~IVoltage() {}

    virtual auto get_samples() const -> std::vector<Sample> const& = 0;

    //filter helpers
    static const size_t FILTER_CHANNELS = 1;
    static void get_channels(double channels[FILTER_CHANNELS], Value const& value)
    {
        channels[0] = value;
    }
    static void get_value(Value& value, double const channels[FILTER_CHANNELS])
    {
        value = channels[0];
    }
};


}
}
}
