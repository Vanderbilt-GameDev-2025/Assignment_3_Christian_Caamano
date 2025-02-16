#include "magneticbody3d.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

using namespace godot;

void MagneticBody3D::_bind_methods() {
    // Property bindings
    ClassDB::bind_method(D_METHOD("set_magnet_type", "type"), &MagneticBody3D::set_magnet_type);
    ClassDB::bind_method(D_METHOD("get_magnet_type"), &MagneticBody3D::get_magnet_type);

    ClassDB::bind_method(D_METHOD("set_strength", "strength"), &MagneticBody3D::set_strength);
    ClassDB::bind_method(D_METHOD("get_strength"), &MagneticBody3D::get_strength);

    // Register properties for the editor
    ADD_PROPERTY(PropertyInfo(Variant::INT, "magnet_type", PROPERTY_HINT_ENUM, "Permanent,Temporary,Electromagnet"), "set_magnet_type", "get_magnet_type");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "strength"), "set_strength", "get_strength");
}

bool MagneticBody3D::willBeInfluencedBy(const MagneticBody3D& other) {
    // If the other magnet is off, no influence.
    if (!other.on) {
        return false;
    }
    
    // Get the vector from this magnet to the other.
    Vector3 distance = other.get_global_position() - get_global_position();
    
    // Check if this magnet is within the other's sphere of influence.
    return distance.length_squared() <= other.maxInfluenceRadiusSqr;
}

void MagneticBody3D::_ready() {
    // Initialize properties specific to this object's magnet type.
    switch (magnetType) {
        case MagnetTypes::Permanent:
            on = true;
            break;
        case MagnetTypes::Temporary:
            on = true;
            break;
        case MagnetTypes::Electromagnet:
            on = false;
            break;
        default:
            UtilityFunctions::print("Object magnet type not set to a valid type in _ready!");
            break;
    }

    // Define influence radius according to the magnet's strength.
    maxInfluenceRadiusSqr = strength * 10.0;
}

// TODO: add additional method implementations

// Getters and setters

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