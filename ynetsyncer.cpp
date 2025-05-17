//
// Created by Daniel on 2024-04-02.
//

#include "ynetsyncer.h"


void YNetPropertySyncer::_bind_methods() {

}

YNetPropertySyncer::YNetPropertySyncer(): property_syncer_index(0), target(), property(), net_id(0), current_val(), authority(0), sync_always(false) {
}

uint32_t YNetPropertySyncer::get_property_syncer_id_from_property_stringnames(const Vector<StringName> &p_property) {
    String combine_p_property;
    for (const auto & p : p_property) {
        combine_p_property += p;
    }
    return YNet::string_to_hash_id(combine_p_property);
}

void YNetPropertySyncer::set_current_val(const Variant &new_value) {
    current_val = new_value;
    Node *actual_node = Object::cast_to<Node>(ObjectDB::get_instance(target));
    if (actual_node != nullptr) {
        actual_node->set_indexed(property, new_value);
    }
}

bool YNetPropertySyncer::check_for_changed_value() {
    if (const Node *actual_node = Object::cast_to<Node>(ObjectDB::get_instance(target)); actual_node != nullptr) {
        Variant current_property_val = actual_node->get_indexed(property);
        if(!current_val.hash_compare(current_property_val)) {
            current_val = current_property_val;
            return true;
        }
    } else {
        print_line(vformat("Check for changed value on netpropertysyncer missing node netid %d property index %d",net_id,property_syncer_index));
    }
    return false;
}

YNetPropertySyncer::YNetPropertySyncer(int p_net_id, Object *p_target, const Vector<StringName> &p_property, const Variant &p_val, int p_authority, bool p_sync_always) {
    net_id = p_net_id;
    if (p_target != nullptr) {
        target = p_target->get_instance_id();
    }
    property = p_property;
    authority = p_authority;
    auto ynet_singleton = YNet::get_singleton();

    if (ynet_singleton == nullptr) return;

    // If the outer_key doesn't exist in the outer HashMap, it would default construct an inner Vector
    Vector<Ref<YNetPropertySyncer>>& inner_map = ynet_singleton->networked_property_syncers[net_id];
    property_syncer_index = inner_map.size();

    inner_map.push_back(this);

    // print_line("Added property syncer, new size is ",ynet_singleton->networked_property_syncers[net_id].size());

    current_val = p_val;

    if (ynet_singleton->queued_received_property_syncers.has(net_id) && ynet_singleton->queued_received_property_syncers[net_id].has(property_syncer_index)) {
            current_val = ynet_singleton->queued_received_property_syncers[net_id][property_syncer_index];
            p_target->set_indexed(property,current_val);
            ynet_singleton->queued_received_property_syncers[net_id].erase(property_syncer_index);
    }
    sync_always = p_sync_always;
}

YNetPropertySyncer::~YNetPropertySyncer() {
    auto ynet_singleton = YNet::get_singleton();
    if (ynet_singleton == nullptr) return;
    if (ynet_singleton->networked_property_syncers.has(net_id)) {
        ynet_singleton->networked_property_syncers[net_id].erase(this);
        if (ynet_singleton->networked_property_syncers[net_id].size() == 0) {
            ynet_singleton->networked_property_syncers.erase(net_id);
        }
    }
}