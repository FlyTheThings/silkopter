#include "BrainStdAfx.h"
#include "RC_Comms.h"
#include "utils/Timed_Scope.h"


namespace silk
{

constexpr uint8_t SDN_GPIO = 6;
constexpr uint8_t NIRQ_GPIO = 5;
constexpr char const* SPI_DEVICE = "/dev/spidev1.0";
constexpr size_t SPEED = 2000000;


RC_Comms::RC_Comms(HAL& hal)
    : m_hal(hal)
    , m_rc_phy(false)
    , m_rc_protocol(m_rc_phy, std::bind(&RC_Comms::process_rx_packet, this, std::placeholders::_1))
    , m_video_streamer()
{
}

auto RC_Comms::start(std::string const& interface, uint8_t id) -> bool
{
    try
    {
        util::comms::Video_Streamer::TX_Descriptor descriptor;
        descriptor.interface = "wlan1";
        descriptor.coding_k = 12;
        descriptor.coding_n = 20;

        m_is_connected = m_rc_phy.init(SPI_DEVICE, SPEED, SDN_GPIO, NIRQ_GPIO)
                        && m_video_streamer.init_tx(descriptor);
    }
    catch(std::exception e)
    {
        m_is_connected = false;
    }

    if (!m_is_connected)
    {
        QLOGW("Cannot start comms on interface {}", interface);
        return false;
    }

    //m_rc_phy.set_rate(100);
    m_rc_protocol.add_periodic_packet(std::chrono::milliseconds(30), std::bind(&RC_Comms::compute_multirotor_state_packet, this, std::placeholders::_1, std::placeholders::_2));

    QLOGI("Started sending on interface {}", interface);

    return true;
}

auto RC_Comms::is_connected() const -> bool
{
    return m_is_connected;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t RC_Comms::compute_multirotor_state_packet(uint8_t* data, uint8_t& packet_type)
{
    packet_type = static_cast<uint8_t>(rc_comms::Packet_Type::MULTIROTOR_STATE);

    size_t off = 0;
    util::serialization::serialize(m_serialization_buffer, m_multirotor_state, off);

    memcpy(data, m_serialization_buffer.data(), off);

    return off;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RC_Comms::process_rx_packet(util::comms::RC_Protocol::RX_Packet const& packet)
{
    m_rx_packet.rx_dBm = packet.rx_dBm;
    m_rx_packet.tx_dBm = packet.tx_dBm;
    m_rx_packet.rx_timepoint = packet.rx_timepoint;

    if (packet.packet_type == static_cast<uint8_t>(rc_comms::Packet_Type::MULTIROTOR_COMMANDS))
    {
        size_t off = 0;
        stream::IMultirotor_Commands::Value value;
        if (util::serialization::deserialize(packet.payload, value, off))
        {
            std::lock_guard<std::mutex> lg(m_samples_mutex);
            m_multirotor_commands = value;
        }
        else
        {
            QLOGW("Cannot deserialize incoming multirotor state value");
        }
    }
    else
    {
        QLOGW("Unknown incoming packet type: {}", static_cast<int>(packet.packet_type));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

boost::optional<stream::IMultirotor_Commands::Value> const& RC_Comms::get_multirotor_commands() const
{
    std::lock_guard<std::mutex> lg(m_samples_mutex);
    return m_multirotor_commands;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RC_Comms::set_multirotor_state(stream::IMultirotor_State::Value const& value)
{
    std::lock_guard<std::mutex> lg(m_samples_mutex);
    m_multirotor_state = value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RC_Comms::add_video_data(stream::IVideo::Value const& value)
{
    m_video_streamer.send(value.data.data(), value.data.size(), value.resolution);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void RC_Comms::process()
{
    if (!is_connected())
    {
        return;
    }

    m_multirotor_commands = boost::none;
}

}
