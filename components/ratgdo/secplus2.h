#pragma once

#include "SoftwareSerial.h" // Using espsoftwareserial https://github.com/plerup/espsoftwareserial
#include "esphome/core/optional.h"

#include "ratgdo_state.h"
#include "protocol.h"
#include "callbacks.h"
#include "observable.h"
#include "common.h"

namespace esphome {

    class Scheduler;
    class InternalGPIOPin;

namespace ratgdo {
    class RATGDOComponent;

namespace secplus2 {

    static const uint8_t PACKET_LENGTH = 19;
    typedef uint8_t WirePacket[PACKET_LENGTH];

    ENUM(CommandType, uint16_t,
        (UNKNOWN, 0x000),
        (GET_STATUS, 0x080),
        (STATUS, 0x081),
        (OBST_1, 0x084), // sent when an obstruction happens?
        (OBST_2, 0x085), // sent when an obstruction happens?
        (PAIR_3, 0x0a0),
        (PAIR_3_RESP, 0x0a1),

        (LEARN_2, 0x181),
        (LOCK, 0x18c),
        (DOOR_ACTION, 0x280),
        (LIGHT, 0x281),
        (MOTOR_ON, 0x284),
        (MOTION, 0x285),

        (LEARN_1, 0x391),
        (PING, 0x392),
        (PING_RESP, 0x393),

        (PAIR_2, 0x400),
        (PAIR_2_RESP, 0x401),
        (SET_TTC, 0x402), // ttc_in_seconds = (byte1<<8)+byte2
        (CANCEL_TTC, 0x408), // ?
        (TTC, 0x40a), // Time to close
        (GET_OPENINGS, 0x48b),
        (OPENINGS, 0x48c), // openings = (byte1<<8)+byte2
    )

    inline bool operator==(const uint16_t cmd_i, const CommandType& cmd_e) { return cmd_i == static_cast<uint16_t>(cmd_e); }
    inline bool operator==(const CommandType& cmd_e, const uint16_t cmd_i) { return cmd_i == static_cast<uint16_t>(cmd_e); }


    struct Command {
        CommandType type;
        uint8_t nibble;
        uint8_t byte1;
        uint8_t byte2;

        Command(): type(CommandType::UNKNOWN) {}
        Command(CommandType type_, uint8_t nibble_ = 0, uint8_t byte1_ = 0, uint8_t byte2_ = 0) : type(type_), nibble(nibble_), byte1(byte1_), byte2(byte2_) {}
    };

    class Secplus2 : public Protocol {
    public:
        void setup(RATGDOComponent* ratgdo, Scheduler* scheduler, InternalGPIOPin* rx_pin, InternalGPIOPin* tx_pin);
        void loop();
        void dump_config();

        void sync();

        void light_action(LightAction action);
        void lock_action(LockAction action);
        void door_action(DoorAction action);
        void query_action(QueryAction action);

        ProtocolArgs call(ProtocolArgs args);

        void increment_rolling_code_counter(int delta = 1);
        void set_rolling_code_counter(uint32_t counter);
        observable<uint32_t>& get_rolling_code_counter();
        void set_client_id(uint64_t client_id);

    protected:
        optional<Command> read_command();
        void handle_command(const Command& cmd);

        void send_command(Command cmd, bool increment = true);
        void send_command(Command cmd, bool increment, std::function<void()>&& on_sent);
        void encode_packet(Command cmd, WirePacket& packet);
        bool transmit_packet();

        void door_command(DoorAction action);

        void print_packet(const char* prefix, const WirePacket& packet) const;
        optional<Command> decode_packet(const WirePacket& packet) const;

        observable<uint32_t> rolling_code_counter_ { 0 };
        uint64_t client_id_ { 0x539 };

        bool transmit_pending_ { false };
        uint32_t transmit_pending_start_ { 0 };
        WirePacket tx_packet_;
        OnceCallbacks<void()> command_sent_;

        SoftwareSerial sw_serial_;

        InternalGPIOPin* tx_pin_;
        InternalGPIOPin* rx_pin_;

        RATGDOComponent* ratgdo_;
        Scheduler* scheduler_;
    };
} // namespace secplus2
} // namespace ratgdo
} // namespace esphome