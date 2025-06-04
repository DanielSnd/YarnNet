#include "ynet_messages.h"

YNetMessage::YNetMessage() {
    type = 0;
}

PackedByteArray YNetMessage::serialize() const {
    PackedByteArray result;
    result.resize(1); // Space for type
    result.write[0] = type;
    
    // Encode data as a variant
    int len = 0;
    encode_variant(data, nullptr, len, false);
    result.resize(1 + len);
    encode_variant(data, &result.write[1], len, false);
    print_line(vformat("[YNet] Serialized message type %d with data %s. Result: %s", type, data, result));
    return result;
}

bool YNetMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    
    // Decode data as a variant
    int len = 0;
    Variant data_var;
    Error err = decode_variant(data_var, ptr + 1, p_data.size() - 1, &len, false);
    if (err != OK) return false;
    data = data_var;
    return true;
}

PackedByteArray YNetJoinRoomWithPasswordMessage::serialize() const {
    PackedByteArray result;
    result.resize(1); // Space for type
    result.write[0] = type;
    
    // Encode room code and password as variants
    int len1 = 0;
    int len2 = 0;
    Variant room_code_var = roomCode;
    Variant password_var = password;
    encode_variant(room_code_var, nullptr, len1, false);
    encode_variant(password_var, nullptr, len2, false);
    
    result.resize(1 + len1 + len2);
    uint8_t *ptr = &result.write[1];
    encode_variant(room_code_var, ptr, len1, false);
    ptr += len1;
    encode_variant(password_var, ptr, len2, false);
    
    return result;
}

bool YNetJoinRoomWithPasswordMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    ptr++;
    
    // Decode room code and password as variants
    int len = 0;
    Variant room_code_var;
    Error err = decode_variant(room_code_var, ptr, p_data.size() - 1, &len, false);
    if (err != OK) return false;
    roomCode = room_code_var;
    ptr += len;
    
    Variant password_var;
    err = decode_variant(password_var, ptr, p_data.size() - 1 - len, &len, false);
    if (err != OK) return false;
    password = password_var;
    
    return true;
}

PackedByteArray YNetConfirmRoomJoinMessage::serialize() const {
    PackedByteArray result;
    result.resize(1); // Space for type
    result.write[0] = type;
    
    // Encode room code and players JSON as variants
    int len = 0;
    int len2 = 0;
    Variant room_code_var = roomCode;
    Variant players_var = jsonRoomPlayers;
    encode_variant(room_code_var, nullptr, len, false);
    encode_variant(players_var, nullptr, len2, false);
    
    result.resize(1 + len + len2);
    uint8_t *ptr = &result.write[1];
    encode_variant(room_code_var, ptr, len, false);
    ptr += len;
    encode_variant(players_var, ptr, len2, false);
    
    return result;
}

bool YNetConfirmRoomJoinMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    ptr++;
    
    // Decode room code and players JSON as variants
    int len = 0;
    Variant room_code_var;
    Error err = decode_variant(room_code_var, ptr, p_data.size() - 1, &len, false);
    if (err != OK) return false;
    roomCode = room_code_var;
    ptr += len;
    
    Variant players_var;
    err = decode_variant(players_var, ptr, p_data.size() - 1 - len, &len, false);
    if (err != OK) return false;
    jsonRoomPlayers = players_var;
    
    return true;
}

PackedByteArray YNetPacketMessage::serialize() const {
    PackedByteArray result;
    // Calculate total size needed: 1 (type) + 1 (channel) + 1 (reliability) + 4 (targetClientId) + 4 (packetData size) + packetData.size()
    result.resize(11 + packetData.size());
    result.write[0] = type; // 1
    result.write[1] = channel; // 1
    result.write[2] = reliability; // 1
    encode_uint32(targetClientId, &result.write[3]); // 4
    encode_uint32(packetData.size(), &result.write[7]); // 4
    memcpy(result.ptrw() + 11, &packetData[0], packetData.size());

    return result;
}

bool YNetPacketMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    channel = ptr[1];
    reliability = ptr[2];
    targetClientId = decode_uint32(ptr + 3);
    packetData.resize(decode_uint32(ptr + 7));
    memcpy(packetData.ptrw(), &p_data[11], packetData.size());
    return true;
}

PackedByteArray YNetRoomSettingMessage::serialize() const {
    PackedByteArray result;
    result.resize(1); // Space for type
    result.write[0] = type;
    
    // Encode setting value as a variant
    int len = 0;
    Variant setting_var = settingValue;
    encode_variant(setting_var, nullptr, len, false);
    
    result.resize(1 + len);
    encode_variant(setting_var, &result.write[1], len, false);
    
    return result;
}

bool YNetRoomSettingMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    ptr++;
    
    // Decode setting value as a variant
    int len = 0;
    Variant setting_var;
    Error err = decode_variant(setting_var, ptr, p_data.size() - 1, &len, false);
    if (err != OK) return false;
    settingValue = setting_var;
    
    return true;
} 

PackedByteArray YNetConfirmConnectionMessage::serialize() const {
    PackedByteArray result;
    result.resize(3); // Space for type
    result.write[0] = type;
    encode_uint16(clientId, &result.write[1]);
    return result;
}

bool YNetConfirmConnectionMessage::deserialize(const PackedByteArray &p_data) {
    if (p_data.size() < 1) return false;
    const uint8_t *ptr = p_data.ptr();
    type = ptr[0];
    ptr++;
    clientId = decode_uint16(ptr);
    return true;
} 