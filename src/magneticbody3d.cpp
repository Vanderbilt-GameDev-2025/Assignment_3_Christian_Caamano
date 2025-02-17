#include "magneticbody3d.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>

using namespace godot;

// --- Class initialization and destruction ---

// Initialize static registry
std::vector<MagneticBody3D*> MagneticBody3D::sceneMagnetsRegistry;

MagneticBody3D::~MagneticBody3D() {
    // Remove magnet from static registry
    unregister_magnet(this);
}


// --- Godot bindings ---

void MagneticBody3D::_bind_methods() {
    // Property bindings
    BIND_ENUM_CONSTANT(Permanent);
    BIND_ENUM_CONSTANT(Temporary);
    BIND_ENUM_CONSTANT(Electromagnet);
    
    ClassDB::bind_method(D_METHOD("set_magnet_type", "type"), &MagneticBody3D::set_magnet_type);
    ClassDB::bind_method(D_METHOD("get_magnet_type"), &MagneticBody3D::get_magnet_type);

    ClassDB::bind_method(D_METHOD("set_strength", "strength"), &MagneticBody3D::set_strength);
    ClassDB::bind_method(D_METHOD("get_strength"), &MagneticBody3D::get_strength);

    // Register properties for the editor
    ADD_PROPERTY(PropertyInfo(Variant::INT, "magnet_type", PROPERTY_HINT_ENUM, "Permanent,Temporary,Electromagnet"), "set_magnet_type", "get_magnet_type");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "strength"), "set_strength", "get_strength");
}


// --- Core methods / magnetic physics calculations ---

bool MagneticBody3D::willBeInfluencedBy(const MagneticBody3D& other) {
    // If the other magnet is off, no influence.
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
    // Calculate distance between this magnet and the other.
    Vector3 thisPos = get_global_position();
    Vector3 otherPos = other.get_global_position();
    Vector3 distanceVec = otherPos - thisPos;
    float distance = distanceVec.length();
    
    // Get pole orientations (using forward vector)
    Vector3 thisOrientation = get_global_transform().basis.get_column(2).normalized();
    Vector3 otherOrientation = other.get_global_transform().basis.get_column(2).normalized();
    
    // Calculate force magnitude using inverse square law
    float forceMagnitude = (strength * other.get_strength()) / (distance * distance);
    
    // Calculate alignment factor (-1 to 1)
    float alignment = thisOrientation.dot(otherOrientation);
    
    // Modify force based on alignment (opposite poles attract)
    forceMagnitude *= -alignment;
    
    // Apply scaling factor
    forceMagnitude *= 10.0f;
    
    // Return force vector
    return distanceVec.normalized() * forceMagnitude;
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
    maxInfluenceRadiusSqr = strength * strength * 100.0;

    // Register this magnet with the static collection of all magnets in the scene.
    register_magnet(this);
}

void MagneticBody3D::_physics_process(double delta) {
    // If this magnet is currently off, it should be excluded from magnetism calculations.
    if (!on) return;

    // Apply forces to this magnet.
    for (const auto& otherMagnet : sceneMagnetsRegistry) {
        if (otherMagnet != this) {
            if (willBeInfluencedBy(*otherMagnet)) {
                if (magnetType == Temporary) {
                    magnetized = true;
                }
                Vector3 force = calculate_force_from_magnet(*otherMagnet);
                apply_central_force(force);
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