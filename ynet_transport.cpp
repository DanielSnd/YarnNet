#include "ynet_transport.h"

void YNetTransport::_bind_methods() {

    BIND_ENUM_CONSTANT(STATE_CLOSED);
    BIND_ENUM_CONSTANT(STATE_CONNECTING);
    BIND_ENUM_CONSTANT(STATE_OPEN);
    BIND_ENUM_CONSTANT(STATE_CLOSING);
}

YNetTransport::State YNetTransport::get_current_state() const {
    if (YNet::get_singleton()->get_offline_mode()) {
        return STATE_OPEN;
    }
    return status;
}

void YNetTransport::set_current_state(YNetTransport::State val) {
        if (status != val) {
            status = val;
            if (status == STATE_CONNECTING) {
                tick_started_connecting = OS::get_singleton()->get_ticks_msec() * 0.001f;
            }
            if (status != STATE_OPEN) {
                YNet::get_singleton()->room_id = "";
            }
            if (YNet::get_singleton()->get_debugging() >= 2) {
                switch ((State)status) {
                    case STATE_CONNECTING:
                        print_line("[YNet] Status is now: Connecting");
                        break;
                    case STATE_OPEN:
                        print_line("[YNet] Status is now: Connected");
                        break;
                    case STATE_CLOSING:
                        print_line("[YNet] Status is now: Closing");
                        break;
                    case STATE_CLOSED:
                        print_line("[YNet] Status is now: Closed");
                        break;
                    default: ;
                }
            }
            YNet::get_singleton()->emit_signal(SNAME("status_changed"),val);
        }
    }


uint32_t YNetTransport::string_to_hash_id(const String &p_string) {
        /* simple djb2 hashing */

        const char32_t *chr = p_string.get_data();
        uint32_t hashv = 5381;
        uint32_t c = *chr++;

        while (c) {
            hashv = (((hashv) << 5) + hashv) + c; /* hash * 33 + c */
            c = *chr++;
        }

        hashv = hash_fmix32(hashv);
        hashv = hashv & 0x7FFFFFFF; // Make it compatible with unsigned, since negative ID is used for exclusion
        return hashv;
}

YNetTransport::YNetTransport() {
}

YNetTransport::~YNetTransport() {
} 