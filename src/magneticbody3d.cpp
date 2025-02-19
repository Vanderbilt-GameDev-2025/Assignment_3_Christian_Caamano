#include "magneticbody3d.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <cmath>

using namespace godot;

// --- Class initialization and destruction ---

// Initialize static registry
std::vector<MagneticBody3D*> MagneticBody3D::sceneMagnetsRegistry;

MagneticBody3D::~MagneticBody3D() {
    // Remove magnet from the static registry
    unregister_magnet(this);
}


// --- Godot bindings ---

void MagneticBody3D::_bind_methods() {
    // Magnet type enum
    BIND_ENUM_CONSTANT(Permanent);
    BIND_ENUM_CONSTANT(Temporary);
    BIND_ENUM_CONSTANT(Electromagnet);
    
    // Getters and setters
    ClassDB::bind_method(D_METHOD("set_magnet_type", "type"), &MagneticBody3D::set_magnet_type);
    ClassDB::bind_method(D_METHOD("get_magnet_type"), &MagneticBody3D::get_magnet_type);

    ClassDB::bind_method(D_METHOD("set_strength", "strength"), &MagneticBody3D::set_strength);
    ClassDB::bind_method(D_METHOD("get_strength"), &MagneticBody3D::get_strength);

    ClassDB::bind_method(D_METHOD("set_on", "on"), &MagneticBody3D::set_on);
    ClassDB::bind_method(D_METHOD("get_on"), &MagneticBody3D::get_on);

    // Expose magnet type and strength to the editor
    ADD_PROPERTY(PropertyInfo(Variant::INT, "magnet_type", PROPERTY_HINT_ENUM, "Permanent,Temporary,Electromagnet"), "set_magnet_type", "get_magnet_type");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "strength"), "set_strength", "get_strength");
}


// --- Core methods / magnetic physics calculations ---

bool MagneticBody3D::will_be_influenced_by(const MagneticBody3D& other) {
    // If the other magnet is off, it exerts no influence.
    if (!other.on) {
        return false;
    }

    // If this is a temporary magnet and the other is also a temporary magnet,
    // the other should only potentially exert a force if it has become magnetized.
    if (magnetType == Temporary && other.magnetType == Temporary) {
        if (other.magnetized == false) {
            return false;
        }
    }
    
    // Get the vector from this magnet to the other.
    Vector3 distance = other.get_global_position() - get_global_position();
    
    // Check if this magnet is within the other's sphere of influence.
    return distance.length_squared() <= other.maxInfluenceRadiusSqr;
}

Vector3 MagneticBody3D::calculate_force_from_magnet(const MagneticBody3D& other) const {
    // The calculation below is roughly based on real magnetism formulas.
    // Essentially, the inverse square law is used to calculate force magnitude based on proximity.
    // This is then scaled based on the magnets' strengths, their relative alignments, and a custom scaling factor.
    // Force direction is determined by the distance vector between the two magnets.

    // Define scaling factor to make forces visible in-game.
    const double FORCE_SCALING = 100.0;

    // Get the length of the distance between the two magnets.
    Vector3 r = other.get_global_position() - get_global_position();
    double r_len = r.length();
    
    // Establish a minimum distance length to prevent division by zero and too large forces at very small distances.
    if (r_len < 0.01) r_len = 0.01;
    
    // Get the unit vector of the distance between the magnets.
    // This will determine the axis of the final force vector.
    Vector3 r_hat = r / r_len;
    
    // Get the forward (local Z) directions of both magnets.
    Vector3 m1_dir = get_global_transform().basis.get_column(2).normalized();
    Vector3 m2_dir = other.get_global_transform().basis.get_column(2).normalized();
    
    // Calculate alignment factor (-1 to 1) to determine whether attraction or repulsion will occur, and at what strength.
    double alignment = m1_dir.dot(m2_dir);
    
    // Determine the final magnitude of the force experienced by this magnet.
    double force_magnitude = FORCE_SCALING * strength * other.strength * alignment / (r_len * r_len);
    
    // Put together and return the final, scaled force vector.
    return r_hat * force_magnitude;
}

Vector3 MagneticBody3D::calculate_torque_from_magnet(const MagneticBody3D& other) const {
    // The calculation below is roughly based on real magnetism formulas.
    // Essentially, the inverse square law is used to calculate torque magnitude based on proximity.
    // This is then scaled based on the magnets' strengths and a custom scaling factor.
    // Torque direction is determined by the relative pole orientations of the magnets.

    // Define scaling factor to make torque visible in-game.
    const double TORQUE_SCALING = 10.0;
    
    // Get the length of the distance between the two magnets.
    Vector3 r = other.get_global_position() - get_global_position();
    double r_len = r.length();
    
    // Establish a minimum distance length to prevent division by zero and too large torques at very small distances.
    if (r_len < 0.1) r_len = 0.1;
    
    // Get the forward (local Z) directions of both magnets.
    Vector3 m1_dir = get_global_transform().basis.get_column(2).normalized();
    Vector3 m2_dir = other.get_global_transform().basis.get_column(2).normalized();
    
    // Calculate torque direction.
    Vector3 torque = m1_dir.cross(m2_dir);
    
    // Determine the final magnitude of the torque experienced by this magnet.
    double torque_magnitude = TORQUE_SCALING * strength * other.strength / (r_len * r_len);
    
    // Put together and return the final, scaled torque vector.
    return torque * torque_magnitude;
}

void MagneticBody3D::_ready() {
    // At the start of the scene, establish the following environment:
    // - Permanent magnets are on
    // - Temporary magnets are on, but start off not magnetized
    // - Electromagnets are off (until turned on on-demand in-game)
    on = true;
    magnetized = false;
    if (magnetType == Electromagnet) {
        on = false;
    }

    // Define influence radius according to the magnet's strength.
    maxInfluenceRadiusSqr = strength * strength * 500.0;

    // Register this magnet with the static collection of all magnets in the scene.
    register_magnet(this);
}

void MagneticBody3D::_physics_process(double delta) {
    // If this magnet is currently off, it should be excluded from magnetism calculations.
    if (!on) return;

    // It is assumed that temporary magnets are currently not magnetized, until influenced by another magnet in the code below.
    magnetized = false;

    // Apply forces to this magnet.
    for (const auto& otherMagnet : sceneMagnetsRegistry) {
        if (otherMagnet != this) {
            if (will_be_influenced_by(*otherMagnet)) {
                if (magnetType == Temporary) {
                    magnetized = true;
                }
                Vector3 force = calculate_force_from_magnet(*otherMagnet);
                Vector3 torque = calculate_torque_from_magnet(*otherMagnet);

                apply_central_force(force);
                apply_torque(torque);
            }
        }
    }
}


// --- Magnet registry management ---

void MagneticBody3D::register_magnet(MagneticBody3D* magnet) {
    if (std::find(sceneMagnetsRegistry.begin(), sceneMagnetsRegistry.end(), magnet) == sceneMagnetsRegistry.end()) {
        sceneMagnetsRegistry.push_back(magnet);
    }
}

void MagneticBody3D::unregister_magnet(MagneticBody3D* magnet) {
    auto registryElement = std::find(sceneMagnetsRegistry.begin(), sceneMagnetsRegistry.end(), magnet);
    if (registryElement != sceneMagnetsRegistry.end()) {
        sceneMagnetsRegistry.erase(registryElement);
    }
}


// --- Getters and setters ---
// Magnet registry
std::vector<MagneticBody3D*>& MagneticBody3D::get_magnets_registry() {
    return sceneMagnetsRegistry;
}

// Magnet type
MagneticBody3D::MagnetTypes MagneticBody3D::get_magnet_type() const {
    return magnetType;
}
void MagneticBody3D::set_magnet_type(const MagnetTypes type) {
    magnetType = type;
}

// Strength
double MagneticBody3D::get_strength() const {
    return strength;
}
void MagneticBody3D::set_strength(const double newStrength) {
    strength = newStrength;
}

// Influence radius
double MagneticBody3D::get_max_influence_radius_sqr() {
    return maxInfluenceRadiusSqr;
}

// Activation state
bool MagneticBody3D::get_on() {
    return on;
}
void MagneticBody3D::set_on(bool newState) {
    on = newState;
}