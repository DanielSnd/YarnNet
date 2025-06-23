#ifndef YSNAPSYNCER_H
#define YSNAPSYNCER_H

#include "scene/main/node.h"
#include "core/math/vector3.h"
#include "core/math/quaternion.h"
#include "core/object/ref_counted.h"
#include "core/templates/list.h"
#include "core/math/math_funcs.h"
#include "core/string/print_string.h"
#include "scene/3d/node_3d.h"

class Node3D;

class YSnapSyncer3D : public Node {
    GDCLASS(YSnapSyncer3D, Node);

public:
    struct Snapshot {
        float interpolation_time;    // Time to interpolate to this snapshot
        Vector3 position;           // World position
        Vector3 velocity;           // Linear velocity
        Quaternion rotation;        // World rotation
        Vector3 angular_velocity;   // Angular velocity in degrees
        Variant extra_data;         // Additional custom data

        Snapshot() : interpolation_time(0.0f), position(Vector3(0.0,0.0,0.0)), velocity(Vector3(0.0,0.0,0.0)), 
                    rotation(Quaternion(0.0,0.0,0.0,1.0)), angular_velocity(Vector3(0.0,0.0,0.0)), extra_data(Variant()) {}
        
        Snapshot(float p_interpolation_time, const Vector3 &p_position, const Vector3 &p_velocity,
                const Quaternion &p_rotation, const Vector3 &p_angular_velocity, const Variant &p_extra_data)
            : interpolation_time(p_interpolation_time), position(p_position), velocity(p_velocity),
              rotation(p_rotation), angular_velocity(p_angular_velocity), extra_data(p_extra_data) {}
    };

    YSnapSyncer3D();

    // Snapshot buffer management
    bool has_snapshots() const { return !snapshots.is_empty(); }
    Snapshot get_last_received_snapshot() const;
    Snapshot get_current_snapshot() const;
    bool is_extrapolating() const { return snapshots.size() == 1; }
    
    // GDScript-exposed snapshot methods (return Dictionary)
    Dictionary get_last_snapshot_dict() const;
    Dictionary get_current_snapshot_dict() const;
    
    // Snapshot addition methods
    void add_snapshot(float interpolation_time, const Vector3 &position, const Quaternion &rotation = Quaternion(), const Variant &extra_data = Variant());
    void add_velocity_snapshot(float interpolation_time, const Vector3 &position, const Vector3 &velocity, const Quaternion &rotation = Quaternion(), const Vector3 &angular_velocity = Vector3(), const Variant &extra_data = Variant());
    
    // Update the playback state
    void update_playback(float delta_time);

    // Apply current interpolated transform to a Node3D
    void apply_to_node(Node3D *p_node);

    // Current interpolated values
    Vector3 get_position() const;
    Vector3 get_velocity() const;
    Quaternion get_rotation() const;
    Vector3 get_angular_velocity() const;
    Variant get_extra_data() const;

    // Configuration accessors
    float get_interpolation_latency() const { return buffer_duration; }
    void set_interpolation_latency(float p_latency) { buffer_duration = p_latency; }
    
    float get_error_correction_speed() const { return drift_correction_rate; }
    void set_error_correction_speed(float p_speed) { drift_correction_rate = p_speed; }
    
    float get_time_correction_speed() const { return time_sync_rate; }
    void set_time_correction_speed(float p_speed) { time_sync_rate = p_speed; }
    
    bool get_disable_extrapolation() const { return no_prediction; }
    void set_disable_extrapolation(bool p_disable) { no_prediction = p_disable; }

    // State information
    float get_playback_time() const { return current_buffer_time; }
    float get_time_drift() const { return accumulated_time_offset; }
    Vector3 get_extrapolation_position_drift() const { return position_correction; }
    Quaternion get_extrapolation_rotation_drift() const { return rotation_correction; }

protected:
    static void _bind_methods();

    // Configuration parameters
    float buffer_duration = 0.12f;           // Buffer length in seconds
    float drift_correction_rate = 10.0f;     // How fast to correct prediction errors
    float time_sync_rate = 3.0f;             // How fast to correct time drift
    bool no_prediction = false;              // Disable future prediction

    void _notification(int p_what);
    
    // Internal state tracking
    float current_buffer_time = 0.0f;        // Current time within buffer
    float accumulated_time_offset = 0.0f;    // Accumulated time offset
    Vector3 position_correction = Vector3(0.0,0.0,0.0);     // Position correction offset
    Quaternion rotation_correction = Quaternion(0.0,0.0,0.0,1.0); // Rotation correction offset
    
    List<Snapshot> snapshots;                // Circular buffer of snapshots

    // Internal calculation methods
    Vector3 calculate_raw_position() const;
    Quaternion calculate_raw_rotation() const;
    Quaternion compute_rotation_delta(const Quaternion &from_rotation, const Quaternion &to_rotation) const;
    Vector3 normalize_euler_angles(const Vector3 &euler_rotation) const;
    float normalize_angle(float angle) const;
};

#endif // YSNAPSYNCER_H
