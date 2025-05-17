#ifndef YNET_TYPES_H
#define YNET_TYPES_H

namespace YNetTypes {
    struct Packet {
        int source = 0;
        uint8_t *data = nullptr;
        uint32_t size = 0;
    };
}

#endif // YNET_TYPES_H 