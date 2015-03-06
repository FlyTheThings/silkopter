#pragma once

#include "common/node/processor/IProcessor.h"

namespace silk
{
namespace node
{
namespace processor
{

class IMultirotor_Pilot : public IProcessor
{
    DEFINE_RTTI_CLASS(IMultirotor_Pilot, IProcessor);
public:
    virtual auto get_type() const -> q::rtti::class_id { return q::rtti::get_class_id<IMultirotor_Pilot>(); }

    enum class Mode : uint8_t
    {
        IDLE,
        ARMED,
        MOTOR_TEST
    };

    struct Input
    {
        struct Toggles
        {
            union
            {
                struct
                {
                    uint8_t take_off    : 1;
                    uint8_t land        : 1;
                    uint8_t lights_on   : 1;
                };
                uint8_t _all = 0;
            };
        };

        enum class Throttle_Mode : uint8_t
        {
            RATE,       //throttle rate of change, per second. 1 means full throttle in one second
            OFFSET,     //-1 .. 1 are offsets from current throttle. Zero means keep same throttle
            ASSISTED,   //climb speed, meters per second
        };

        enum class Pitch_Roll_Mode : uint8_t
        {
            RATE,       //angle rate of change - radians per second
            HORIZONTAL, //angle from horizontal. zero means horizontal
            ASSISTED,   //speed, meters per second
        };

        enum class Yaw_Mode : uint8_t
        {
            RATE,
        };

        //the reference frame for the user controls
        enum class Reference_Frame : uint8_t
        {
            LOCAL, 	//normal mode - back means back of uav
            USER,	//simple mode - back means towards the user, front away from her.
        };

        struct Assists
        {
            union
            {
                struct
                {
                    uint8_t stay_in_radio_range     : 1; //avoid out of radio range situations. Configured by Params::max_radio_distance
                    uint8_t stay_in_battery_range   : 1; //avoid going too far considering current battery.
                    uint8_t stay_in_perimeter       : 1; //stay in a configured perimeter.
                    uint8_t avoid_altitude_drop     : 1; //avoid dropping too much altitude too fast. Controlled by Params::min_time_to_ground parameter.
                    uint8_t avoid_the_user          : 1; //avoid being too low around the home position. Needs sonar and GPS. Controlled by the Params::home_radius
                    uint8_t avoid_the_ground        : 1; //maintains a min distance from the ground. Needs sonar. Configured by Params::min_ground_distance parameter
                };
                uint8_t _all = 0;
            };
        };

        struct Sticks
        {
            float throttle = 0;
            float yaw = 0;
            float pitch = 0;
            float roll = 0;
        };

        ///////////////////////////////
        /// Data

        Toggles toggles;
        Sticks sticks;
        Throttle_Mode throttle_mode = Throttle_Mode::OFFSET;
        Pitch_Roll_Mode pitch_roll_mode = Pitch_Roll_Mode::HORIZONTAL;
        Yaw_Mode yaw_mode = Yaw_Mode::RATE;
        Reference_Frame reference_frame = Reference_Frame::LOCAL;
        Assists assists;

        math::vec3f camera_mount_rotation;
    };

    virtual void set_mode(Mode mode) const = 0;
    virtual auto get_mode() const -> Mode = 0;

    virtual void set_input(Input const& input) = 0;
    virtual auto get_input() const -> Input = 0;
};



}
}
}