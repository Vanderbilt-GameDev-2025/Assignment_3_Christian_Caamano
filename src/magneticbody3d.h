#ifndef MAGNETIC_BODY_3D_H
#define MAGNETIC_BODY_3D_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>

namespace godot {

/**
 * Extends the physics engine (Jolt) to support magnetic objects in a 3D environment.
 * Features/capabilities:
 * 1. Realistic dipole physics and interactions (differing attraction/repulsion depending on orientation & position)
 * 2. Permanent magnets: objects that are always magnets
 * 3. Temporary magnets: objects that temporarily become magnets when in the presence of a magnetic field
 * 4. Electromagnets (kind of): magnetism can be turned on/off on demand for these objects, simulating electromagnetism
 */
class MagneticBody3D : public RigidBody3D{
    GDCLASS(MagneticBody3D, RigidBody3D)

private:
    /**
     * The different types of magnets supported.
     */
    enum class MagnetTypes {
        Permanent,
        Temporary,
        Electromagnet
    };

    /**
     * The magnet type for this object.
     */
    MagnetTypes magnetType;

    /**
     * Defines whether this magnet is currently on or off.
     * Tip: Magnets that are off do not undergo any magnetism calculations, saving on performance costs.
     */
    bool on;

    // TODO: add any additional necessary private variables

    /**
     * Defines the strength of this magnet. Stronger magnets exert more attractive/repulsive force.
     */
    double strength;

    /**
     * Defines the square of the radius of the sphere of influence for this magnet, beyond which the magnet will be treated as if it were off.
     * This value is automatically set based on the magnet's strength (i.e., stronger magnets will have a larger radius of influence).
     * To obtain the actual radius of influence, take the square root of this value.
     */
    double maxInfluenceRadiusSqr;

    /**
     * Determine if this magnet will be influenced by another magnet.
     * The other magnet will only exert an influence on this one if this one lies within the other's sphere of influence,
     * as defined by its maxInfluenceRadiusSqr.
     * 
     * @param other The other magnet which may or may not exert an influence on this object.
     */
    bool willBeInfluencedBy(const MagneticBody3D& other);

    // TODO: add any additional necessary private methods

protected:
    /**
     * Bind methods and register properties for the editor.
     */
    static void _bind_methods();
    
public:
    // Default constructor/destructor
    MagneticBody3D() = default;
    ~MagneticBody3D() = default;
    
    /**
     * Called when the node enters the scene tree for the first time.
     * Initializes this magnetic object's properties.
     */
    virtual void _ready() override;

    // TODO: add doc comment here
    virtual void _physics_process(double delta) override;

    // TODO: add any additional necessary public methods

    /**
     * Get the magnet type for this object.
     */
    MagnetTypes get_magnet_type() const;

    /**
     * Set the magnet type for this object.
     */
    void set_magnet_type(const MagnetTypes type);

    /**
     * Get the strength for this object.
     */
    double get_strength() const;

    /**
     * Set the strength for this object.
     */
    void set_strength(const double newStrength);
};

}  // namespace godot

#endif // MAGNETIC_BODY_3D_H