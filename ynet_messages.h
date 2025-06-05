#ifndef YNET_MESSAGES_H
#define YNET_MESSAGES_H

#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "core/io/marshalls.h"
#include "core/object/ref_counted.h"

class YNetMessage : public RefCounted {
    GDCLASS(YNetMessage, RefCounted);

public:

    enum MessageType {
        MESSAGE_ERROR = 0,
        ROOM_CREATE = 1,
        ROOM_JOIN = 2,
        ROOM_LEAVE = 3,
        RPC_TO_SERVER = 4,
        RPC_TO_CLIENTS = 5,
        RPC_TO_CLIENT = 6,
        MESSAGE = 7,
        GET_ROOM_LIST = 8,
        CONFIRM_ROOM_CREATION = 9,
        CONFIRM_ROOM_JOIN = 10,
        PLAYER_JOINED = 11,
        ROOM_PEER_JOIN = 12,
        ROOM_PEER_LEFT = 13,
        SET_ROOM_PASSWORD = 14,
        SET_MAX_PLAYERS = 15,
        SET_ROOM_PRIVATE = 16,
        SET_HOST_MIGRATION = 17,
        SET_ROOM_NAME = 18,
        SET_EXTRA_INFO = 19,
        GET_ROOM_INFO = 20,
        PACKET_TO_SERVER = 21,
        PACKET_TO_CLIENTS = 22,
        PACKET_TO_CLIENT = 23,
        KICK_CLIENT = 24,
        JOIN_ROOM_WITH_PASSWORD = 25,
        JOIN_OR_CREATE_ROOM = 26,
        JOIN_OR_CREATE_ROOM_RANDOM = 27,
        PLAYER_LEFT = 28,
        CREATE_ROOM_WITH_PASSWORD = 29,
        HOST_LEFT = 30,
        CONFIRM_CONNECTION = 31,
        NOT_MESSAGE = 255
    };


    MessageType type;
    uint16_t roomId;
    String data;

    YNetMessage();

    // Serialization methods
    virtual PackedByteArray serialize() const;
    virtual bool deserialize(const PackedByteArray &p_data);
};

class YNetJoinRoomWithPasswordMessage : public YNetMessage {
    GDCLASS(YNetJoinRoomWithPasswordMessage, YNetMessage);

public:
    String roomCode;
    String password;

    YNetJoinRoomWithPasswordMessage() {
        type = YNetMessage::JOIN_ROOM_WITH_PASSWORD;
    }

    virtual PackedByteArray serialize() const override;
    virtual bool deserialize(const PackedByteArray &p_data) override;
};

class YNetConfirmRoomJoinMessage : public YNetMessage {
    GDCLASS(YNetConfirmRoomJoinMessage, YNetMessage);


public:
    String roomCode;
    String jsonRoomPlayers;

    YNetConfirmRoomJoinMessage() {
        type = YNetMessage::CONFIRM_ROOM_JOIN;
    }

    virtual PackedByteArray serialize() const override;
    virtual bool deserialize(const PackedByteArray &p_data) override;
};

class YNetPacketMessage : public YNetMessage {
    GDCLASS(YNetPacketMessage, YNetMessage);

public:
    uint8_t reliability;
    uint8_t channel;
    uint32_t targetClientId;
    PackedByteArray packetData;

    YNetPacketMessage() : targetClientId(0) {
        type = YNetMessage::PACKET_TO_SERVER;
        channel = 0;
        reliability = 1;
    }

    virtual PackedByteArray serialize() const override;
    virtual bool deserialize(const PackedByteArray &p_data) override;
};

class YNetRoomSettingMessage : public YNetMessage {
    GDCLASS(YNetRoomSettingMessage, YNetMessage);

public:
    String settingValue;

    YNetRoomSettingMessage() {
        type = YNetMessage::SET_ROOM_PASSWORD;
    }

    virtual PackedByteArray serialize() const override;
    virtual bool deserialize(const PackedByteArray &p_data) override;
};

class YNetConfirmConnectionMessage : public YNetMessage {
    GDCLASS(YNetConfirmConnectionMessage, YNetMessage);

public:
    uint16_t clientId;

    YNetConfirmConnectionMessage() {
        type = YNetMessage::CONFIRM_CONNECTION;
    }

    virtual PackedByteArray serialize() const override;
    virtual bool deserialize(const PackedByteArray &p_data) override;
};
#endif // YNET_MESSAGES_H 