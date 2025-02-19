extends Node3D

# Array to store references to electromagnets
var coils: Array[MagneticBody3D] = []

# Reference to projectile scene to spawn projectiles
@export var projectile_scene: PackedScene
@export var spawn_point: Node3D

# Physics-based timing settings
@export var physics_steps_per_activation: int = 2  # How many physics steps each coil stays active
@export var max_projectile_speed: float = 100.0    # Maximum allowed projectile speed

# Projectile lifetime in seconds (set to 0 to disable auto-removal)
@export var projectile_lifetime: float = 1.0

# Firing sequence state
var is_firing: bool = false
var current_projectile: MagneticBody3D = null
var current_coil_index: int = -1      # -1 indicates no active coil
var steps_remaining: int = 0          # Steps remaining for current coil

"""
Initialize coil array at the beginning of the scene.
"""
func _ready():
    # Get all child electromagnets and store them in order
    for child in get_children():
        if child is MagneticBody3D and child.get_magnet_type() == MagneticBody3D.Electromagnet:
            coils.append(child)
    
    # Sort coils by their Z position to ensure proper sequence
    coils.sort_custom(func(a, b): return a.global_position.z > b.global_position.z)

"""
Process calculations in time with the physics engine.
"""
func _physics_process(_delta):
    if not is_firing:
        return
        
    if current_projectile:
        # Clamp projectile velocity to prevent extreme physics responses
        if current_projectile.linear_velocity.length() > max_projectile_speed:
            current_projectile.linear_velocity = current_projectile.linear_velocity.normalized() * max_projectile_speed
        
        # Debug velocity
        print_debug("Projectile Velocity: ", current_projectile.linear_velocity)
    
    # Handle coil timing
    if steps_remaining > 0:
        steps_remaining -= 1
        if steps_remaining == 0:
            deactivate_current_coil()
    elif current_coil_index < coils.size() - 1:
        # Activate next coil
        activate_next_coil()
    else:
        # All coils have fired
        end_firing_sequence()

"""
Spawn a projectile into the scene.
"""
func spawn_projectile() -> MagneticBody3D:
    if not projectile_scene:
        print_debug("ERROR: No projectile scene set!")
        return null
    
    # Instance the projectile scene
    var instance = projectile_scene.instantiate() as MagneticBody3D
    if not instance:
        print_debug("ERROR: Failed to instantiate projectile or invalid type!")
        return null
    
    # Add it to the scene
    get_tree().root.add_child(instance)
    
    # Position it at the spawn point or behind first coil
    if spawn_point:
        instance.global_transform = spawn_point.global_transform
    elif coils.size() > 0:
        var spawn_position = coils[0].global_position
        spawn_position.z -= 1.0  # Adjust this offset as needed
        instance.global_position = spawn_position

    # Reset projectile physics state
    instance.linear_velocity = Vector3.ZERO
    instance.angular_velocity = Vector3.ZERO
    
    # Give physics time to stabilize
    instance.set_deferred("freeze", true)
    # Wait for 2 physics frames
    await get_tree().physics_frame
    await get_tree().physics_frame
    instance.set_deferred("freeze", false)
    await get_tree().physics_frame  # Wait one more frame before proceeding

    # Schedule the projectile for removal after its lifetime expires (if set)
    if projectile_lifetime > 0:
        var timer = get_tree().create_timer(projectile_lifetime)
        timer.timeout.connect(instance.queue_free)

    print_debug("Spawned new projectile")
    return instance

"""
Activate the next coil in the sequence.
"""
func activate_next_coil():
    current_coil_index += 1
    if current_coil_index < coils.size():
        var coil = coils[current_coil_index]
        if is_projectile_valid_for_coil(coil):
            print_debug("Activating coil ", current_coil_index)
            coil.set_on(true)
            steps_remaining = physics_steps_per_activation
        else:
            # Skip this coil if projectile isn't in valid position
            activate_next_coil()

"""
Deactivate the current coil.
"""
func deactivate_current_coil():
    if current_coil_index >= 0 and current_coil_index < coils.size():
        print_debug("Deactivating coil ", current_coil_index)
        coils[current_coil_index].set_on(false)

"""
Check if the projectile is within the appropriate proximity of the next coil to activate.
"""
func is_projectile_valid_for_coil(coil: MagneticBody3D) -> bool:
    if not current_projectile:
        return false
    # Check if projectile is in a valid position relative to coil
    var to_projectile = current_projectile.global_position - coil.global_position
    var forward = -coil.global_transform.basis.z
    return to_projectile.dot(forward) < 0

"""
Fire the coilgun if it is not currently in a firing sequence and a projectile has been loaded.
"""
func fire_coilgun():
    if not is_firing:
        print_debug("=== Starting Coilgun Firing Sequence ===")
        reset_coilgun()
        
        # Spawn new projectile
        current_projectile = await spawn_projectile()
        if current_projectile:
            start_firing_sequence()
        else:
            print_debug("Failed to spawn projectile!")

"""
Begin the firing sequence.
"""
func start_firing_sequence():
    is_firing = true
    current_coil_index = -1
    steps_remaining = 0
    activate_next_coil()

"""
End the firing sequence.
"""
func end_firing_sequence():
    print_debug("=== Firing Sequence Complete ===")
    is_firing = false
    deactivate_current_coil()
    current_coil_index = -1
    steps_remaining = 0

"""
Reset the coilgun prior to the next firing sequence.
"""
func reset_coilgun():
    is_firing = false
    current_coil_index = -1
    steps_remaining = 0
    
    # Deactivate all coils
    for coil in coils:
        coil.set_on(false)
