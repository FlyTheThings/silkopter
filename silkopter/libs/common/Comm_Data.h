#pragma once

namespace silk
{
namespace comms
{

enum class Telemetry_Message : uint8_t
{
    //------------------------
    //streams
    //These have to be enabled with a Setup_Message::STREAM_TELEMETRY request

    //RES:
    // uint32_t size - array size
    // Stream::Sample[size] - the stream samples. The sample type depends on the message
    ACCELERATION_STREAM,
    ADC_VALUE_STREAM,
    ANGULAR_VELOCITY_STREAM,
    BATTERY_STATE_STREAM,
    CARDINAL_POINTS_STREAM,
    CURRENT_STREAM,
    DISTANCE_STREAM,
    LINEAR_ACCELERATION_STREAM,
    LOCATION_STREAM,
    MAGNETIC_FIELD_STREAM,
    PRESSURE_STREAM,
    PWM_VALUE_STREAM,
    REFERENCE_FRAME_STREAM,
    TEMPERATURE_STREAM,
    VOLTAGE_STREAM,
    VIDEO_STREAM,
};

enum class Sim_Message : uint8_t
{
    SENSORS,
    MOTORS,
};


enum class Setup_Message : uint8_t
{
    //returns all the sources in the system
    //REQ: - no arguments
    //RES:
    //  string - source name
    //  uint8_t output_count - array size
    //  string[output_count] - name of all output streams
    ENUMERATE_SOURCES,

    //returns all the streams in the system
    //REQ: - no arguments
    //RES:
    //  string - stream name
    ENUMERATE_STREAMS,

    //returns all the processors in the system
    //REQ: - no arguments
    //RES:
    //  string - processor name
    //  uint8_t input_count - array size
    //  string[input_count] - name of all input streams
    //  uint8_t output_count - array size
    //  string[output_count] - name of all output streams
    ENUMERATE_PROCESSORS,

    //returns all the sinks in the system
    //REQ: - no arguments
    //RES:
    //  string - sink name
    //  uint8_t input_count - array size
    //  string[input_count] - name of all input streams
    ENUMERATE_SINKS,

    //sets/gets a node config as a json string
    //REQ:
    //  [optional] string - if present, the config to set. If not set - it's a request
    //RES:
    //  string - the config
    SOURCE_CONFIG,
    STREAM_CONFIG,
    PROCESSOR_CONFIG,
    SINK_CONFIG,

    //start or stop telemetry streaming.
    //REQ:
    //  uint8_t size - array size
    //  string[size] - names of the streams
    //RES:
    //  uint8_t size - array size
    //  string[size] - names of the streams that will be sent as telemetry
    STREAM_TELEMETRY,

    //-----------------------------
    // Multirotor

    //Set/Get the mode
    //REQ:
    //  [optional] Multirotor::Mode - if present, the mode to change to. If not it's a getter
    //REQ:
    //  Multirotor::Mode - the current mode
    MULTIROTOR_MODE,

    //Get the input
    //REQ: - no arguments
    //REQ:
    //  Multirotor::Input - the current input values
    REQUEST_MULTIROTOR_INPUT,

//    // Plane
//    AIRPLANE_MODE,
//    REQUEST_AIRPLANE_INPUT,

//    // Rover
//    ROVER_MODE,
//    REQUEST_ROVER_INPUT,

//    // Boat
//    BOAT_MODE,
//    REQUEST_BOAT_INPUT,
};

enum class Input_Message : uint8_t
{
    MULTIROTOR_INPUT,
    AIRPLANE_INPUT,
    ROVER_INPUT,
    BOAT_INPUT,
};



}
}
