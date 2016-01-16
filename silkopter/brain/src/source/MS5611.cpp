#include "BrainStdAfx.h"
#include "MS5611.h"

#include "sz_math.hpp"
#include "sz_MS5611.hpp"

namespace silk
{
namespace node
{


//registers
constexpr uint8_t CMD_MS5611_RESET = 0x1E;
constexpr uint8_t CMD_MS5611_PROM_Setup = 0xA0;
constexpr uint8_t CMD_MS5611_PROM_C1 = 0xA2;
constexpr uint8_t CMD_MS5611_PROM_C2 = 0xA4;
constexpr uint8_t CMD_MS5611_PROM_C3 = 0xA6;
constexpr uint8_t CMD_MS5611_PROM_C4 = 0xA8;
constexpr uint8_t CMD_MS5611_PROM_C5 = 0xAA;
constexpr uint8_t CMD_MS5611_PROM_C6 = 0xAC;
constexpr uint8_t CMD_MS5611_PROM_CRC = 0xAE;

constexpr uint8_t CMD_CONVERT_D1_OSR256 = 0x40;
constexpr uint8_t CMD_CONVERT_D1_OSR512 = 0x42;
constexpr uint8_t CMD_CONVERT_D1_OSR1024 = 0x44;
constexpr uint8_t CMD_CONVERT_D1_OSR2048 = 0x46;
constexpr uint8_t CMD_CONVERT_D1_OSR4096 = 0x48;

constexpr uint8_t CMD_CONVERT_D2_OSR256 = 0x50;
constexpr uint8_t CMD_CONVERT_D2_OSR512 = 0x52;
constexpr uint8_t CMD_CONVERT_D2_OSR1024 = 0x54;
constexpr uint8_t CMD_CONVERT_D2_OSR2048 = 0x56;
constexpr uint8_t CMD_CONVERT_D2_OSR4096 = 0x58;

constexpr uint8_t ADDR_MS5611 = 0x77;

constexpr uint8_t READ_FLAG = 0x80;


MS5611::MS5611(HAL& hal)
    : m_hal(hal)
    , m_init_params(new sz::MS5611::Init_Params())
    , m_config(new sz::MS5611::Config())
{
    m_pressure = std::make_shared<Pressure_Stream>();
    m_temperature = std::make_shared<Temperature_Stream>();
}

auto MS5611::lock(Buses& buses) -> bool
{
    if (buses.i2c)
    {
        buses.i2c->lock(); //lock the bus
        return true;
    }
    if (buses.spi)
    {
        buses.spi->lock(); //lock the bus
        return true;
    }
    return false;
}
void MS5611::unlock(Buses& buses)
{
    if (buses.i2c)
    {
        buses.i2c->unlock(); //unlock the bus
        return;
    }
    if (buses.spi)
    {
        buses.spi->unlock(); //unlock the bus
        return;
    }
}

auto MS5611::bus_read(Buses& buses, uint8_t reg, uint8_t* rx_data, uint32_t size) -> bool
{
    m_dummy_tx_data.resize(size);
    return buses.i2c ? buses.i2c->read_register(ADDR_MS5611, reg, rx_data, size)
         : buses.spi ? buses.spi->transfer_register(reg | READ_FLAG, m_dummy_tx_data.data(), rx_data, size)
         : false;
}
auto MS5611::bus_read_u8(Buses& buses, uint8_t reg, uint8_t& rx_data) -> bool
{
    uint8_t dummy_data = 0;
    return buses.i2c ? buses.i2c->read_register_u8(ADDR_MS5611, reg, rx_data)
         : buses.spi ? buses.spi->transfer_register_u8(reg | READ_FLAG, dummy_data, rx_data)
         : false;
}
auto MS5611::bus_read_u16(Buses& buses, uint8_t reg, uint16_t& rx_data) -> bool
{
    uint16_t dummy_data = 0;
    return buses.i2c ? buses.i2c->read_register_u16(ADDR_MS5611, reg, rx_data)
         : buses.spi ? buses.spi->transfer_register_u16(reg | READ_FLAG, dummy_data, rx_data)
         : false;
}
auto MS5611::bus_write(Buses& buses, uint8_t reg, uint8_t const* tx_data, uint32_t size) -> bool
{
    m_dummy_rx_data.resize(size);
    return buses.i2c ? buses.i2c->write_register(ADDR_MS5611, reg, tx_data, size)
         : buses.spi ? buses.spi->transfer_register(reg, tx_data, m_dummy_rx_data.data(), size)
         : false;
}
auto MS5611::bus_write_u8(Buses& buses, uint8_t reg, uint8_t const& tx_data) -> bool
{
    uint8_t dummy_data = 0;
    return buses.i2c ? buses.i2c->write_register_u8(ADDR_MS5611, reg, tx_data)
         : buses.spi ? buses.spi->transfer_register_u8(reg, tx_data, dummy_data)
         : false;
}
auto MS5611::bus_write_u16(Buses& buses, uint8_t reg, uint16_t const& tx_data) -> bool
{
    uint16_t dummy_data = 0;
    return buses.i2c ? buses.i2c->write_register_u16(ADDR_MS5611, reg, tx_data)
         : buses.spi ? buses.spi->transfer_register_u16(reg, tx_data, dummy_data)
         : false;
}

auto MS5611::get_outputs() const -> std::vector<Output>
{
    std::vector<Output> outputs(2);
    outputs[0].name = "Pressure";
    outputs[0].stream = m_pressure;
    outputs[1].name = "Temperature";
    outputs[1].stream = m_temperature;
    return outputs;
}
auto MS5611::init(rapidjson::Value const& init_params) -> bool
{
    QLOG_TOPIC("ms5611::init");

    sz::MS5611::Init_Params sz;
    autojsoncxx::error::ErrorStack result;
    if (!autojsoncxx::from_value(sz, init_params, result))
    {
        std::ostringstream ss;
        ss << result;
        QLOGE("Cannot deserialize MS5611 data: {}", ss.str());
        return false;
    }
    *m_init_params = sz;
    return init();
}
auto MS5611::init() -> bool
{
    m_i2c = m_hal.get_buses().find_by_name<bus::II2C>(m_init_params->bus);
    m_spi = m_hal.get_buses().find_by_name<bus::ISPI>(m_init_params->bus);

    Buses buses = { m_i2c.lock(), m_spi.lock() };
    if (!buses.i2c && !buses.spi)
    {
        QLOGE("No bus configured");
        return false;
    }

    lock(buses);
    At_Exit at_exit([this, &buses]()
    {
        unlock(buses);
    });

    m_init_params->pressure_rate = math::clamp<size_t>(m_init_params->pressure_rate, 10, 100);
    m_init_params->temperature_rate_ratio = math::clamp<size_t>(m_init_params->temperature_rate_ratio, 1, 10);

    QLOGI("Probing MS5611 on {}", m_init_params->bus);

    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    bus_write(buses, CMD_MS5611_RESET, nullptr, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // We read the factory calibration
    // The on-chip CRC is not used
    uint16_t C1, C2, C3, C4, C5, C6;
    auto res = bus_read_u16(buses, CMD_MS5611_PROM_C1, C1);
    res &= bus_read_u16(buses, CMD_MS5611_PROM_C2, C2);
    res &= bus_read_u16(buses, CMD_MS5611_PROM_C3, C3);
    res &= bus_read_u16(buses, CMD_MS5611_PROM_C4, C4);
    res &= bus_read_u16(buses, CMD_MS5611_PROM_C5, C5);
    res &= bus_read_u16(buses, CMD_MS5611_PROM_C6, C6);
    if (!res)
    {
        QLOGE("MS5611 not found!");
#ifdef RASPBERRY_PI
        return false;
#endif
    }
    QLOGI("PROM: {} {} {} {} {} {}", C1, C2, C3, C4, C5, C6);

    m_c1 = C1;
    m_c2 = C2;
    m_c3 = C3;
    m_c4 = C4;
    m_c5 = C5;
    m_c6 = C6;
    if (m_c1 == 0 || m_c2 == 0 || m_c3 == 0 || m_c4 == 0 || m_c5 == 0 || m_c6 == 0)
    {
        QLOGE("MS5611 seems broken!");
#ifdef RASPBERRY_PI
        return false;
#endif
    }

    bus_write(buses, CMD_CONVERT_D2_OSR256, nullptr, 0);

    m_pressure->set_rate(m_init_params->pressure_rate);
    m_temperature->set_rate(m_init_params->pressure_rate / m_init_params->temperature_rate_ratio);

    return true;
}

auto MS5611::start(q::Clock::time_point tp) -> bool
{
    m_last_reading_tp = tp;
    m_pressure->set_tp(tp);
    m_temperature->set_tp(tp);
    return true;
}

void MS5611::process()
{
    m_pressure->clear();
    m_temperature->clear();

    Buses buses = { m_i2c.lock(), m_spi.lock() };
    if (!buses.i2c && !buses.spi)
    {
        return;
    }

    lock(buses);
    At_Exit at_exit([this, &buses]()
    {
        unlock(buses);
    });

    QLOG_TOPIC("ms5611::process");
    auto now = q::Clock::now();
    if (now - m_last_reading_tp < m_pressure->get_dt())
    {
        return;
    }

    std::array<uint8_t, 3> buf;
    if (m_stage == 0)
    {
        //read temperature
        if (bus_read(buses, 0x00, buf.data(), buf.size()))
        {
            double val = (((uint32_t)buf[0]) << 16) | (((uint32_t)buf[1]) << 8) | buf[2];
            m_temperature->reading = val;

        }

        //next
        if (bus_write(buses, CMD_CONVERT_D1_OSR256, nullptr, 0)) //read pressure next
        {
            m_stage++;
        }
    }
    else
    {
        //read pressure
        if (bus_read(buses, 0x00, buf.data(), buf.size()))
        {
            double val = (((uint32_t)buf[0]) << 16) | (((uint32_t)buf[1]) << 8) | buf[2];
            m_pressure->reading = val;
        }

        //next
        if (m_stage >= m_init_params->temperature_rate_ratio)
        {
            if (bus_write(buses, CMD_CONVERT_D2_OSR256, nullptr, 0)) //read temp next
            {
                m_stage = 0;
            }
        }
        else if (bus_write(buses, CMD_CONVERT_D1_OSR256, nullptr, 0)) //read pressure next
        {
            m_stage++;
        }
    }
    m_last_reading_tp = now;

    {
        // Formulas from manufacturer datasheet
        // sub -20c temperature compensation is not included

        double dT = m_temperature->reading - m_c5 * 256.0;
        double TEMP = 2000.0 + (dT * m_c6)*0.00000011920928955078125;
        double OFF = m_c2 * 65536.0 + (m_c4 * dT) * 0.0078125;
        double SENS = m_c1 * 32768.0 + (m_c3 * dT) * 0.00390625;

        if (TEMP < 2000.0)
        {
            // second order temperature compensation when under 20 degrees C
            double T2 = (dT*dT) / 0x80000000;
            double Aux = TEMP*TEMP;
            double OFF2 = 2.5*Aux;
            double SENS2 = 1.25*Aux;
            TEMP = TEMP - T2;
            OFF = OFF - OFF2;
            SENS = SENS - SENS2;
        }

        float t = static_cast<float>(TEMP) * 0.01f;
        double p = (m_pressure->reading*SENS*0.000000476837158203125 - OFF)*0.000030517578125;

        {
            auto samples_needed = m_pressure->compute_samples_needed();
            while (samples_needed > 0)
            {
                m_pressure->push_sample(p, true);
                samples_needed--;
            }
        }
        {
            auto samples_needed = m_temperature->compute_samples_needed();
            while (samples_needed > 0)
            {
                m_temperature->push_sample(t, true);
                samples_needed--;
            }
        }
    }
}

auto MS5611::set_config(rapidjson::Value const& json) -> bool
{
    QLOG_TOPIC("ms5611::set_config");

    sz::MS5611::Config sz;
    autojsoncxx::error::ErrorStack result;
    if (!autojsoncxx::from_value(sz, json, result))
    {
        std::ostringstream ss;
        ss << result;
        QLOGE("Cannot deserialize MS5611 config data: {}", ss.str());
        return false;
    }

    *m_config = sz;
    return true;
}
auto MS5611::get_config() const -> rapidjson::Document
{
    rapidjson::Document json;
    autojsoncxx::to_document(*m_config, json);
    return std::move(json);
}

auto MS5611::get_init_params() const -> rapidjson::Document
{
    rapidjson::Document json;
    autojsoncxx::to_document(*m_init_params, json);
    return std::move(json);
}

auto MS5611::send_message(rapidjson::Value const& /*json*/) -> rapidjson::Document
{
    return rapidjson::Document();
}

}
}
