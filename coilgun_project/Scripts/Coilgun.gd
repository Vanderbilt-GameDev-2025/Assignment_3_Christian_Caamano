extends Node3D

# Array to store references to electromagnet nodes
var coils: Array[MagneticBody3D] = []

# Reference to projectile scene for spawning
@export var projectile_scene: PackedScene

# Reference to spawn point (can be a Node3D marker)
@export var spawn_point: Node3D

# Timing settings
var activation_delay: float = 0.01  # Delay between activating each coil
var active_duration: float = 0.04  # How long each coil stays active

# Flag to track if coilgun is currently firing
var is_firing: bool = false

# Currently active projectile
var current_projectile: MagneticBody3D = null

func _ready():
    # Get all child electromagnets and store them in order
    for child in get_children():
        if child is MagneticBody3D and child.get_magnet_type() == MagneticBody3D.Electromagnet:
            coils.append(child)
            
    # Sort coils by their Z position to ensure proper sequence
    coils.sort_custom(func(a, b): return a.global_position.z > b.global_position.z)

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
    
    # Position it at the spawn point if one is set, otherwise at the first coil
    if spawn_point:
        instance.global_transform = spawn_point.global_transform
    elif coils.size() > 0:
        # Position slightly behind first coil
        var spawn_position = coils[0].global_position
        spawn_position.z -= 1.0  # Adjust this offset as needed
        instance.global_position = spawn_position
    
    print_debug("Spawned new projectile")
    return instance

func fire_coil_sequence():
    is_firing = true
    
    # Schedule activation and deactivation for each coil
    for i in range(coils.size()):
        var activation_time = i * activation_delay
        var deactivation_time = activation_time + active_duration
        
        # Schedule activation
        get_tree().create_timer(activation_time).timeout.connect(
            func(): activate_coil(i)
        )
        
        # Schedule deactivation
        get_tree().create_timer(deactivation_time).timeout.connect(
            func(): deactivate_coil(i)
        )
    
    # Schedule the end of firing sequence
    var sequence_duration = (coils.size() * activation_delay) + active_duration
    get_tree().create_timer(sequence_duration).timeout.connect(
        func(): end_firing_sequence()
    )

func activate_coil(index: int):
    if index >= 0 and index < coils.size():
        print_debug("Activating coil ", index)
        coils[index].set_on(true)

func deactivate_coil(index: int):
    if index >= 0 and index < coils.size():
        print_debug("Deactivating coil ", index)
        coils[index].set_on(false)

func end_firing_sequence():
    is_firing = false
    print_debug("Firing sequence complete")
    # Optionally, you could queue_free() the projectile here if you want to clean it up
    # current_projectile.queue_free()

# Start firing sequence
func fire_coilgun():
    if not is_firing:
        print_debug("=== Starting Coilgun Firing Sequence ===")
        reset_coilgun()
        
        # Spawn new projectile
        current_projectile = spawn_projectile()
        if current_projectile:
            fire_coil_sequence()
        else:
            print_debug("Failed to spawn projectile!")

# Reset coilgun to initial state
func reset_coilgun():
    print_debug("=== Resetting Coilgun ===")
    is_firing = false
    
    # Deactivate all coils
    for coil in coils:
        coil.set_on(false)