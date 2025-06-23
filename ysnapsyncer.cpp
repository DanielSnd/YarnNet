#include "ysnapsyncer.h"

void YSnapSyncer3D::_bind_methods() {
    // Bind snapshot management methods
    ClassDB::bind_method(D_METHOD("has_snapshots"), &YSnapSyncer3D::has_snapshots);
    ClassDB::bind_method(D_METHOD("get_last_snapshot_dict"), &YSnapSyncer3D::get_last_snapshot_dict);
    ClassDB::bind_method(D_METHOD("get_current_snapshot_dict"), &YSnapSyncer3D::get_current_snapshot_dict);
    ClassDB::bind_method(D_METHOD("is_extrapolating"), &YSnapSyncer3D::is_extrapolating);
    
    // Bind snapshot addition methods
    ClassDB::bind_method(D_METHOD("add_snapshot", "interpolation_time", "position", "rotation", "extra_data"), &YSnapSyncer3D::add_snapshot);
    ClassDB::bind_method(D_METHOD("add_velocity_snapshot", "interpolation_time", "position", "velocity", "rotation", "angular_velocity", "extra_data"), &YSnapSyncer3D::add_velocity_snapshot);
    ClassDB::bind_method(D_METHOD("update_playback", "delta_time"), &YSnapSyncer3D::update_playback);

    // Bind utility methods
    ClassDB::bind_method(D_METHOD("apply_to_node", "node"), &YSnapSyncer3D::apply_to_node);
    
    // Bind transform access methods
    ClassDB::bind_method(D_METHOD("get_position"), &YSnapSyncer3D::get_position);
    ClassDB::bind_method(D_METHOD("get_velocity"), &YSnapSyncer3D::get_velocity);
    ClassDB::bind_method(D_METHOD("get_rotation"), &YSnapSyncer3D::get_rotation);
    ClassDB::bind_method(D_METHOD("get_angular_velocity"), &YSnapSyncer3D::get_angular_velocity);
    ClassDB::bind_method(D_METHOD("get_extra_data"), &YSnapSyncer3D::get_extra_data);
    
    // Bind configuration properties
    ClassDB::bind_method(D_METHOD("get_interpolation_latency"), &YSnapSyncer3D::get_interpolation_latency);
    ClassDB::bind_method(D_METHOD("set_interpolation_latency", "latency"), &YSnapSyncer3D::set_interpolation_latency);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "interpolation_latency"), "set_interpolation_latency", "get_interpolation_latency");
    
    ClassDB::bind_method(D_METHOD("get_error_correction_speed"), &YSnapSyncer3D::get_error_correction_speed);
    ClassDB::bind_method(D_METHOD("set_error_correction_speed", "speed"), &YSnapSyncer3D::set_error_correction_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "error_correction_speed"), "set_error_correction_speed", "get_error_correction_speed");
    
    ClassDB::bind_method(D_METHOD("get_time_correction_speed"), &YSnapSyncer3D::get_time_correction_speed);
    ClassDB::bind_method(D_METHOD("set_time_correction_speed", "speed"), &YSnapSyncer3D::set_time_correction_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_correction_speed"), "set_time_correction_speed", "get_time_correction_speed");
    
    ClassDB::bind_method(D_METHOD("get_disable_extrapolation"), &YSnapSyncer3D::get_disable_extrapolation);
    ClassDB::bind_method(D_METHOD("set_disable_extrapolation", "disable"), &YSnapSyncer3D::set_disable_extrapolation);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disable_extrapolation"), "set_disable_extrapolation", "get_disable_extrapolation");
    
    // Bind state information methods
    ClassDB::bind_method(D_METHOD("get_playback_time"), &YSnapSyncer3D::get_playback_time);
    ClassDB::bind_method(D_METHOD("get_time_drift"), &YSnapSyncer3D::get_time_drift);
    ClassDB::bind_method(D_METHOD("get_extrapolation_position_drift"), &YSnapSyncer3D::get_extrapolation_position_drift);
    ClassDB::bind_method(D_METHOD("get_extrapolation_rotation_drift"), &YSnapSyncer3D::get_extrapolation_rotation_drift);
}

YSnapSyncer3D::YSnapSyncer3D() {
    // Initialize with default values (set in header)
}

void YSnapSyncer3D::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        // Node is ready, perform any initialization
    }
    if (p_what == NOTIFICATION_PROCESS) {
        // Update every frame with the process delta time
        update_playback(static_cast<float>(get_process_delta_time()));
    }
}

YSnapSyncer3D::Snapshot YSnapSyncer3D::get_last_received_snapshot() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get last snapshot from empty buffer");
        return Snapshot();
    }
    return snapshots.back()->get();
}

YSnapSyncer3D::Snapshot YSnapSyncer3D::get_current_snapshot() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get current snapshot from empty buffer");
        return Snapshot();
    }
    return snapshots.front()->get();
}

Dictionary YSnapSyncer3D::get_last_snapshot_dict() const {
    Snapshot snapshot = get_last_received_snapshot();
    Dictionary dict;
    dict["interpolation_time"] = snapshot.interpolation_time;
    dict["position"] = snapshot.position;
    dict["velocity"] = snapshot.velocity;
    dict["rotation"] = snapshot.rotation;
    dict["angular_velocity"] = snapshot.angular_velocity;
    dict["extra_data"] = snapshot.extra_data;
    return dict;
}

Dictionary YSnapSyncer3D::get_current_snapshot_dict() const {
    Snapshot snapshot = get_current_snapshot();
    Dictionary dict;
    dict["interpolation_time"] = snapshot.interpolation_time;
    dict["position"] = snapshot.position;
    dict["velocity"] = snapshot.velocity;
    dict["rotation"] = snapshot.rotation;
    dict["angular_velocity"] = snapshot.angular_velocity;
    dict["extra_data"] = snapshot.extra_data;
    return dict;
}

void YSnapSyncer3D::add_snapshot(float interpolation_time, const Vector3 &position, const Quaternion &rotation, const Variant &extra_data) {
    // Delegate to the full snapshot method with default velocities
    add_velocity_snapshot(interpolation_time, position, Vector3(), rotation, Vector3(), extra_data);
}

void YSnapSyncer3D::add_velocity_snapshot(float interpolation_time, const Vector3 &position, const Vector3 &velocity, const Quaternion &rotation, const Vector3 &angular_velocity, const Variant &extra_data) {
    // Ensure minimum interpolation time for first snapshot
    if (snapshots.size() < 1) {
        interpolation_time = MAX(buffer_duration, 0.01f);
    }
    
    // Calculate total time remaining in buffer
    float remaining_buffer_time = interpolation_time - current_buffer_time;
    for (List<Snapshot>::Element *E = snapshots.front(); E; E = E->next()) {
        if (E != snapshots.front()) {
            remaining_buffer_time += E->get().interpolation_time;
        }
    }
    
    // Update time drift
    accumulated_time_offset = buffer_duration - remaining_buffer_time;
    
    // Create initial snapshot if buffer is empty
    if (snapshots.size() < 1) {
        Snapshot fake_snapshot(0.0f, position, Vector3(), rotation, Vector3(), extra_data);
        snapshots.push_back(fake_snapshot);
    }
    
    // Store current state before adding new snapshot
    Vector3 position_before_new_snapshot = get_position();
    Quaternion rotation_before_new_snapshot = get_rotation();
    
    // Calculate velocities from position/rotation differences
    Snapshot last_received_snapshot = get_last_received_snapshot();
    Vector3 calculated_velocity = interpolation_time > 0.0f ? 
        (position - last_received_snapshot.position) / interpolation_time : Vector3();
    
    Quaternion calculated_rotation_difference = compute_rotation_delta(last_received_snapshot.rotation, rotation);
    Vector3 calculated_angular_velocity = interpolation_time > 0.0f ? 
        normalize_euler_angles(calculated_rotation_difference.get_euler()) / interpolation_time : Vector3();
    
    // Create and add the new snapshot
    Snapshot snapshot(interpolation_time, position, 
                     velocity.is_zero_approx() ? calculated_velocity : velocity,
                     rotation,
                     angular_velocity.is_zero_approx() ? calculated_angular_velocity : angular_velocity,
                     extra_data);
    snapshots.push_back(snapshot);
    
    // Update previous snapshot's velocities for smooth interpolation
    if (snapshots.size() >= 2) {
        List<Snapshot>::Element *prev_snapshot = snapshots.back()->prev();
        prev_snapshot->get().velocity = calculated_velocity;
        prev_snapshot->get().angular_velocity = calculated_angular_velocity;
    }
    
    // Process the buffer update
    update_playback(0.0f);
    
    // Calculate drift correction
    Vector3 position_after_new_snapshot = calculate_raw_position();
    Quaternion rotation_after_new_snapshot = calculate_raw_rotation();
    
    position_correction = position_before_new_snapshot - position_after_new_snapshot;
    rotation_correction = compute_rotation_delta(rotation_after_new_snapshot, rotation_before_new_snapshot);
}

void YSnapSyncer3D::update_playback(float delta_time) {
    if (snapshots.size() < 1) {
        // Nothing to update in empty buffer
        return;
    }
    
    if (delta_time > 0.0f) {
        // Gradually correct time drift
        float time_drift_correction = -Math::lerp(0.0f, accumulated_time_offset, time_sync_rate * delta_time);
        delta_time += time_drift_correction;
        accumulated_time_offset += time_drift_correction;
        current_buffer_time += delta_time;
        
        // Smoothly reduce extrapolation errors
        position_correction = position_correction.lerp(Vector3(), drift_correction_rate * delta_time);
        rotation_correction = rotation_correction.slerp(Quaternion(), drift_correction_rate * delta_time);
    }
    
    // Remove expired snapshots from buffer
    while (snapshots.size() > 1 && current_buffer_time >= snapshots.front()->next()->get().interpolation_time) {
        // Reset drift for instant snapshots (teleports)
        if (snapshots.front()->next()->get().interpolation_time == 0.0f) {
            position_correction = Vector3();
            rotation_correction = Quaternion();
        }
        
        current_buffer_time -= snapshots.front()->next()->get().interpolation_time;
        snapshots.pop_front();
    }
}

void YSnapSyncer3D::apply_to_node(Node3D *p_node) {
    ERR_FAIL_NULL(p_node);
    
    // Apply current interpolated position
    p_node->set_global_position(get_position());
    
    // Apply rotation while preserving scale
    Vector3 scale_before = p_node->get_basis().get_scale();
    Basis new_basis = Basis(get_rotation());
    new_basis.scale(scale_before);
    p_node->set_basis(new_basis);
}

Vector3 YSnapSyncer3D::get_position() const {
    // Return position with error correction applied
    return calculate_raw_position() + position_correction;
}

Vector3 YSnapSyncer3D::get_velocity() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get velocity from empty snapshot buffer");
        return Vector3();
    }
    return snapshots.front()->get().velocity;
}

Quaternion YSnapSyncer3D::get_rotation() const {
    // Return rotation with error correction applied
    return calculate_raw_rotation() * rotation_correction;
}

Vector3 YSnapSyncer3D::get_angular_velocity() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get angular velocity from empty snapshot buffer");
        return Vector3();
    }
    return snapshots.front()->get().angular_velocity;
}

Variant YSnapSyncer3D::get_extra_data() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get extra data from empty snapshot buffer");
        return Variant();
    }
    return snapshots.front()->get().extra_data;
}

Vector3 YSnapSyncer3D::calculate_raw_position() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get position from empty snapshot buffer");
        return Vector3();
    }
    
    // If extrapolation is disabled and we're predicting, return current position
    if (no_prediction && is_extrapolating()) {
        return snapshots.front()->get().position;
    }
    
    // Calculate interpolated position using velocity
    return snapshots.front()->get().position + snapshots.front()->get().velocity * current_buffer_time;
}

Quaternion YSnapSyncer3D::calculate_raw_rotation() const {
    if (!has_snapshots()) {
        WARN_PRINT("Attempting to get rotation from empty snapshot buffer");
        return Quaternion();
    }
    
    // If extrapolation is disabled and we're predicting, return current rotation
    if (no_prediction && is_extrapolating()) {
        return snapshots.front()->get().rotation;
    }
    
    // Calculate interpolated rotation using angular velocity
    Vector3 euler_rotation = snapshots.front()->get().angular_velocity * current_buffer_time;
    return snapshots.front()->get().rotation * Quaternion::from_euler(euler_rotation);
}

Quaternion YSnapSyncer3D::compute_rotation_delta(const Quaternion &from_rotation, const Quaternion &to_rotation) const {
    // Calculate the rotation difference between two quaternions
    return from_rotation.inverse() * to_rotation;
}

Vector3 YSnapSyncer3D::normalize_euler_angles(const Vector3 &euler_rotation) const {
    // Convert euler angles to -180 to 180 degree range
    return Vector3(normalize_angle(euler_rotation.x),
                   normalize_angle(euler_rotation.y),
                   normalize_angle(euler_rotation.z));
}

float YSnapSyncer3D::normalize_angle(float angle) const {
    // Convert angle from 0-360 range to -180 to 180 range
    return angle > 180.0f ? angle - 360.0f : angle;
}
