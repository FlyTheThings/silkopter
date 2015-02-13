#pragma once

#include "IStream.h"
#include "Sample.h"

namespace silk
{
namespace node
{
namespace stream
{

class IDistance : public IStream
{
public:
    typedef float             Value; //meters
    typedef Sample<Value>     Sample;

    virtual ~IDistance() {}

    virtual auto get_samples() const -> std::vector<Sample> const& = 0;
};

DECLARE_CLASS_PTR(IDistance);

}
}
}
