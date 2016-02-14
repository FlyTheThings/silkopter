#pragma once

#include "HAL.h"
#include "common/node/ISource.h"
#include "common/stream/IPosition.h"
#include "common/stream/IVelocity.h"
#include "common/stream/IGPS_Info.h"
#include "common/bus/II2C.h"
#include "common/bus/ISPI.h"
#include "common/bus/IUART.h"

#include "Basic_Output_Stream.h"

namespace sz
{
namespace UBLOX
{
struct Init_Params;
struct Config;
}
}


namespace silk
{
namespace node
{

class UBLOX : public ISource
{
public:

    UBLOX(HAL& hal);
    ~UBLOX();

    auto init(rapidjson::Value const& init_params) -> bool;
    auto get_init_params() const -> rapidjson::Document;

    auto set_config(rapidjson::Value const& json) -> bool;
    auto get_config() const -> rapidjson::Document;

    auto send_message(rapidjson::Value const& json) -> rapidjson::Document;

    auto start(q::Clock::time_point tp) -> bool override;

    auto get_outputs() const -> std::vector<Output>;

    void process();

private:
    auto init() -> bool;

    HAL& m_hal;

    bus::II2C_wptr m_i2c;
    bus::ISPI_wptr m_spi;
    bus::IUART_wptr m_uart;

    struct Buses
    {
        bus::II2C_ptr i2c;
        bus::ISPI_ptr spi;
        bus::IUART_ptr uart;
    };
    //i use 2 vectors to avoid memsetting the tx_data to zero all the time
    std::vector<uint8_t> m_dummy_tx_data;
    std::vector<uint8_t> m_dummy_rx_data;

    auto lock(Buses& buses) -> bool;
    void unlock(Buses& buses);

    auto read(Buses& buses, uint8_t* data, size_t max_size) -> size_t;
    auto write(Buses& buses, uint8_t const* data, size_t size) -> bool;

    std::shared_ptr<sz::UBLOX::Init_Params> m_init_params;
    std::shared_ptr<sz::UBLOX::Config> m_config;

    struct Packet
    {
        uint8_t cls;
        uint16_t message;
        std::vector<uint8_t> payload;
    } m_packet;

    auto setup() -> bool;
    void poll_for_data(Buses& buses);
    void reset(Buses& buses);

    void read_data(Buses& buses);

    struct Decoder_State
    {
        enum class Result
        {
            FOUND_PACKET,
            NEEDS_DATA,
            DONE
        };

        size_t data_idx = 0;
        size_t step = 0;
        size_t payload_size = 0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;
    };
    Decoder_State m_state;

    auto decode_packet(Packet& packet, std::deque<uint8_t>& buffer) -> Decoder_State::Result;
    void process_packet(Buses& buses, Packet& packet);

    void process_nav_sol_packet(Buses& buses, Packet& packet);
    void process_nav_velecef_packet(Buses& buses, Packet& packet);
    void process_nav_posecef_packet(Buses& buses, Packet& packet);
    void process_nav_pollh_packet(Buses& buses, Packet& packet);
    void process_nav_status_packet(Buses& buses, Packet& packet);

    void process_cfg_prt_packet(Buses& buses, Packet& packet);
    void process_cfg_rate_packet(Buses& buses, Packet& packet);
    void process_cfg_sbas_packet(Buses& buses, Packet& packet);
    void process_cfg_ant_packet(Buses& buses, Packet& packet);
    void process_cfg_msg_packet(Buses& buses, Packet& packet);

    void process_inf_notice_packet(Buses& buses, Packet& packet);

    void process_mon_hw_packet(Buses& buses, Packet& packet);
    void process_mon_ver_packet(Buses& buses, Packet& packet);

    template<class T> auto send_packet(Buses& buses, uint16_t mgs, T const& payload) -> bool;
    template<class T> auto send_packet_with_retry(Buses& buses, uint16_t msg, T const& data, q::Clock::duration timeout, size_t retries) -> bool;
    auto send_packet(Buses& buses, uint16_t mgs, uint8_t const* payload, size_t payload_size) -> bool;

    auto wait_for_ack(Buses& buses, q::Clock::duration d) -> bool;
    boost::optional<bool> m_ack;

    enum class Setup_State
    {
        UNKNOWN,
        RUNNING,
        FAILED,
        DONE
    };

    std::atomic<Setup_State> m_setup_state = {Setup_State::UNKNOWN};
    std::future<void> m_setup_future;


    std::array<uint8_t, 1024> m_temp_buffer;
    std::deque<uint8_t> m_buffer;

    typedef Basic_Output_Stream<stream::IECEF_Position> Position_Stream;
    mutable std::shared_ptr<Position_Stream> m_position_stream;

    typedef Basic_Output_Stream<stream::IECEF_Velocity> Velocity_Stream;
    mutable std::shared_ptr<Velocity_Stream> m_velocity_stream;

    typedef Basic_Output_Stream<stream::IGPS_Info> GPS_Info_Stream;
    mutable std::shared_ptr<GPS_Info_Stream> m_gps_info_stream;

    Position_Stream::Value m_last_position_value;
    q::Clock::time_point m_last_position_tp = q::Clock::time_point(q::Clock::duration::zero());

    Velocity_Stream::Value m_last_velocity_value;
    q::Clock::time_point m_last_velocity_tp = q::Clock::time_point(q::Clock::duration::zero());

    GPS_Info_Stream::Value m_last_gps_info_value;
    q::Clock::time_point m_last_gps_info_tp = q::Clock::time_point(q::Clock::duration::zero());

    q::Clock::time_point m_last_process_tp = q::Clock::time_point(q::Clock::duration::zero());
    q::Clock::time_point m_last_reset_tp = q::Clock::now() - std::chrono::seconds(1000);
    q::Clock::time_point m_last_poll_tp = q::Clock::now() - std::chrono::seconds(1000);
};


}
}
