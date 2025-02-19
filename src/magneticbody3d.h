#ifndef MAGNETIC_BODY_3D_H
#define MAGNETIC_BODY_3D_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_body3d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>

using namespace godot;

/**
 * Extends the physics engine (Jolt) to support magnetic objects in a 3D environment.
 * Features/capabilities:
 * 1. Realistic dipole physics and interactions (differing attraction/repulsion depending on orientation & position)
 * 2. Permanent magnets: objects that are always magnets
 * 3. Temporary magnets: objects that temporarily become magnets when in the presence of a magnetic field
 * 4. Electromagnets: magnetism can be turned on/off on demand for these objects, simulating electromagnetism
 */
class MagneticBody3D : public RigidBody3D {
    GDCLASS(MagneticBody3D, RigidBody3D)

public:

    // --- Public fields ---

    /**
     * The different types of magnets supported.
     */
    enum MagnetTypes {
        Permanent,
        Temporary,
        Electromagnet
    };
    

    // --- Constructor/destructor ---

    /** Default constructor */ 
    MagneticBody3D() = default;

    /**
     * Destructor; unregisters this magnet from the registry before destruction.
     */
    ~MagneticBody3D();


    // --- Public getters and setters ---

    /**
     * Gets the registry containing references to all the magnets in the scene.
     * 
     * @return A reference to the static magnets registry.
     */
    static std::vector<MagneticBody3D*>& get_magnets_registry();

    /**
     * Gets the magnet type for this magnet.
     * 
     * @return The magnet's magnet type.
     */
    MagnetTypes get_magnet_type() const;

    /**
     * Gets the strength for this magnet.
     * 
     * @return The magnet's strength.
     */
    double get_strength() const;

    /**
     * Gets the square of the radius of the sphere of influence for this magnet.
     * 
     * @return The square of the max influence radius.
     */
    double get_max_influence_radius_sqr();

    /**
     * Gets the activation state for this magnet.
     * 
     * @return True if the magnet is on, false if off.
     */
    bool get_on();

    /**
     * Turns this magnet on or off according to the parameter state.alignas
     * 
     * @param newState The new activation state for this magnet.
     */
    void set_on(bool newState);


    // --- Core magnetism methods ---

    /**
     * Determines if this magnet will be influenced by another magnet.
     * The other magnet will only exert an influence on this one if this one lies within the other's sphere of influence,
     * as defined by its maxInfluenceRadiusSqr.
     * 
     * @param other The other magnet which may or may not exert an influence on this object.
     * @return True if the other magnet will exert an influence on this one, false if not.
     */
    bool will_be_influenced_by(const MagneticBody3D& other);

    /**
     * Calculates the magnetic force exerted on this magnet by another magnet.
     * 
     * @param other The other magnet.
     * @return The vector indicating the central force this magnet will experience.
     */
    Vector3 calculate_force_from_magnet(const MagneticBody3D& other) const;

    /**
     * Calculates the torque exerted on this magnet by another magnet due to their dipoles seeking to align.
     * 
     * @param other The other magnet.
     * @return The vector indicating the torque this magnet will experience.
     */
    Vector3 calculate_torque_from_magnet(const MagneticBody3D& other) const;

    /**
     * Called when the node enters the scene tree for the first time.
     * Initializes this magnetic object's properties.
     */
    virtual void _ready() override;

    /**
     * Accumulates and applies to this magnet the forces exerted by all other magnets in the scene for the current physics frame.
     */
    virtual void _physics_process(double delta) override;

protected:
    /**
     * Binds methods and registers properties for the editor.
     */
    static void _bind_methods();
    
private:

    // --- Private fields ---

    /**
     * The magnet type for this object.
     */
    MagnetTypes magnetType;

    /**
     * Defines whether this magnet is currently on or off.
     * Tip: Magnets that are off do not undergo any magnetism calculations, saving on performance costs.
     */
    bool on;

    /**
     * Defines whether a temporary magnet is currently magnetized or not.
     * Temporary magnets become magnetized when in the presence of another magnetic field.
     */
    bool magnetized;

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
     * Collection containing references to all the magnets in the scene.
     */
    static std::vector<MagneticBody3D*> sceneMagnetsRegistry;


    // --- Private setters ---

    // NOTE: The setters for strength and magnet type are private, as these properties
    // are meant to be invariant during runtime. They can only be changed in the editor.

    /**
     * Sets the strength for this object.
     */
    void set_strength(const double newStrength);

    /**
     * Sets the magnet type for this object.
     */
    void set_magnet_type(const MagnetTypes type);


    // --- Magnet registry methods ---

    /**
     * Adds the specified magnet to the scene registry (sceneMagnetsRegistry).
     * 
     * @param magnet The magnet to add to the registry.
     */
    static void register_magnet(MagneticBody3D* magnet);

    /**
     * Removes the specified magnet from the scene registry (sceneMagnetsRegistry).
     * 
     * @param magnet The magnet to remove from the registry.
     */
    static void unregister_magnet(MagneticBody3D* magnet);
};

// Register MagnetTypes enum with Godot.
VARIANT_ENUM_CAST(MagneticBody3D::MagnetTypes);


#endif // MAGNETIC_BODY_3D_H