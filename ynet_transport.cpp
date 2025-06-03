#include "ynet_transport.h"

void YNetTransport::_bind_methods() {
    ClassDB::bind_method(D_METHOD("connect_to", "address"), &YNetTransport::connect_to);
    ClassDB::bind_method(D_METHOD("disconnect"), &YNetTransport::disconnect);
    ClassDB::bind_method(D_METHOD("send_packet", "data", "size"), &YNetTransport::send_packet);
    ClassDB::bind_method(D_METHOD("poll"), &YNetTransport::poll);
    ClassDB::bind_method(D_METHOD("get_state"), &YNetTransport::get_state);
    ClassDB::bind_method(D_METHOD("has_packet"), &YNetTransport::has_packet);
    ClassDB::bind_method(D_METHOD("get_packet"), &YNetTransport::get_packet);
    ClassDB::bind_method(D_METHOD("set_max_queued_packets", "max_queued_packets"), &YNetTransport::set_max_queued_packets);
    ClassDB::bind_method(D_METHOD("get_max_queued_packets"), &YNetTransport::get_max_queued_packets);
    ClassDB::bind_method(D_METHOD("set_inbound_buffer_size", "size"), &YNetTransport::set_inbound_buffer_size);
    ClassDB::bind_method(D_METHOD("set_outbound_buffer_size", "size"), &YNetTransport::set_outbound_buffer_size);
    ClassDB::bind_method(D_METHOD("get_close_code"), &YNetTransport::get_close_code);
    ClassDB::bind_method(D_METHOD("get_close_reason"), &YNetTransport::get_close_reason);
    ClassDB::bind_method(D_METHOD("get_current_state"), &YNetTransport::get_current_state);
    ClassDB::bind_method(D_METHOD("set_current_state", "state"), &YNetTransport::set_current_state);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "current_state"), "get_current_state", "set_current_state");

    BIND_ENUM_CONSTANT(STATE_CLOSED);
    BIND_ENUM_CONSTANT(STATE_CONNECTING);
    BIND_ENUM_CONSTANT(STATE_OPEN);
    BIND_ENUM_CONSTANT(STATE_CLOSING);
}

YNetTransport::State YNetTransport::get_current_state() const {
    if(YNet::get_singleton()->get_offline_mode()) {
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

YNetTransport::YNetTransport() {
}

YNetTransport::~YNetTransport() {
} 