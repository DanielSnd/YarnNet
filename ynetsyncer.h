//
// Created by Daniel on 2024-04-02.
//

#ifndef YNETSYNCER_H
#define YNETSYNCER_H
#include "scene/main/node.h"
#include "yarnnet.h"
#include "ynet_multiplayer_peer.h"

class YNet;
///
/// YNetSyncer has a list of other nodes it observes. This is set on edit time
/// On ready the YNetSyncer takes inventory of all the properties of the nodes in the list
/// If a property starts with _sync_ it gets added to a list of observable properties it'll sync.
/// Set an interval on it and it'll on that interval check the properties for changes.
/// Those properties get stored in a RefCounted type that has a signal you can query and subscribe to to receive
/// change notifications for the property.
///
/// This could also handle spawning. It could use unique ids, and authority. You'd set that info before adding it to the
/// tree, and when it gets added to the tree it'd then request the other connections to spawn it.
///
/// When a new connection joins all ynet syncers syncronize their info into a big packet that gets sent to the new
/// connection. The new connection needs to aknowledge that packet.
///
/// I think that's mostly it. Could possibly also route rpcs through here maybe. Add some GDVirtual callbacks.
///

class YNetPropertySyncer : public RefCounted {
    GDCLASS(YNetPropertySyncer, RefCounted);

protected:
    static void _bind_methods();

public:
    YNetPropertySyncer();

    uint8_t property_syncer_index;
    ObjectID target;
    Vector<StringName> property;
    int net_id;
    Variant current_val;
    int authority;
    bool sync_always;

    static uint32_t get_property_syncer_id_from_property_stringnames(const Vector<StringName> &p_property);
    void set_current_val(const Variant &new_value);
    bool check_for_changed_value();

    YNetPropertySyncer(int p_net_id, Object *p_target, const Vector<StringName> &p_property, const Variant &p_val, int authority, bool p_sync_always);
    ~YNetPropertySyncer();
};

#endif //YNETSYNCER_H
