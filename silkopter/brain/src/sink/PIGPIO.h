#pragma once

#include "HAL.h"
#include "common/node/ISink.h"
#include "common/stream/IPWM.h"

namespace sz
{
namespace PIGPIO
{
struct Init_Params;
struct Config;
struct PWM_Channel;
struct PWM_Config;
}
}


namespace silk
{
namespace node
{

class PIGPIO : public ISink
{
public:
    PIGPIO(HAL& hal);
    ~PIGPIO();

    static const size_t MAX_PWM_CHANNELS = 8;

    auto init(rapidjson::Value const& init_params) -> bool;
    auto get_init_params() const -> rapidjson::Document;

    auto set_config(rapidjson::Value const& json) -> bool;
    auto get_config() const -> rapidjson::Document;

    auto send_message(rapidjson::Value const& json) -> rapidjson::Document;

    auto start(q::Clock::time_point tp) -> bool override;

    void set_input_stream_path(size_t idx, q::Path const& path);
    auto get_inputs() const -> std::vector<Input>;

    void process();

private:
    auto init() -> bool;

    HAL& m_hal;

    std::shared_ptr<sz::PIGPIO::Init_Params> m_init_params;
    std::shared_ptr<sz::PIGPIO::Config> m_config;

    struct PWM_Channel
    {
        bool is_servo = false;
        uint32_t rate = 0;
        sz::PIGPIO::PWM_Config* config = nullptr;
        uint32_t gpio = 0;
        q::Path stream_path;
        stream::IPWM_wptr stream;
    };

    std::vector<PWM_Channel> m_pwm_channels;

    void set_pwm_value(size_t idx, float value);
};

}
}
