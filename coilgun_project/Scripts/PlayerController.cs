using Godot;
using System;

public partial class PlayerController : CharacterBody3D
{
	// Gravity
	private const float gravity = 9.8f;

	// Movement logic variables
	private float movementSpeed = 15.0f;
	private float frictionFactor = 60.0f;

	// Jump logic variables
	private bool isGrounded = false;
	private float jumpForce = 7.0f;
	private RayCast3D groundCheck;

	// Camera system variables
	private float mouse_sensitivity = 0.0006f;
	private float twistInput = 0.0f;
	private float pitchInput = 0.0f;
	private Node3D twistPivot;
	private Node3D pitchPivot;

	// Interaction logic variables
	private RayCast3D interactionRay;
	
	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		groundCheck = GetNode<RayCast3D>("GroundCheck");
		interactionRay = GetNode<RayCast3D>("TwistPivot/PitchPivot/Camera3D/InteractionRay");

		twistPivot = GetNode<Node3D>("TwistPivot");
		pitchPivot = GetNode<Node3D>("TwistPivot/PitchPivot");

		// Capture and hide the mouse during gameplay.
		Input.MouseMode = Input.MouseModeEnum.Captured;
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		// Pitch and twist the camera based on user mouse movement.
		twistPivot.RotateY(twistInput);
        pitchPivot.RotateX(pitchInput);
		pitchPivot.Rotation = pitchPivot.Rotation with {
			X = Mathf.Clamp(
			pitchPivot.Rotation.X,
			Mathf.DegToRad(-60),
			Mathf.DegToRad(60))
		};

		// Reset camera system variables for next frame.
		twistInput = 0;
		pitchInput = 0;
	}

	public override void _PhysicsProcess(double delta)
	{
		Vector3 newVel = Velocity;
		
		// Jump logic
		if (!IsOnFloor())
		{
			newVel.Y -= gravity * (float)delta;
		}
		else if (Input.IsActionJustPressed("jump"))
        {
            newVel.Y = jumpForce;
        }

		// Calculate movement right, left, back, or forward based on user input.
		// See project Input Map settings for keyboard mappings.
		Vector3 input = Vector3.Zero;
		input.X = Input.GetAxis("move_left", "move_right");
        input.Z = Input.GetAxis("move_forward", "move_back");

		Vector3 direction = twistPivot.Basis * input;

        if (direction != Vector3.Zero)
        {
            newVel.X = direction.X * movementSpeed;
            newVel.Z = direction.Z * movementSpeed;
        }
        else
        {
            newVel.X = Mathf.MoveToward(Velocity.X, 0, movementSpeed);
            newVel.Z = Mathf.MoveToward(Velocity.Z, 0, movementSpeed);
        }

		// Apply movement.
		Velocity = newVel;
		MoveAndSlide();
	}

    public override void _UnhandledInput(InputEvent @event)
    {
        // Player looking around with the mouse
		if (@event is InputEventMouseMotion mouseMotion)
		{
			if (Input.MouseMode == Input.MouseModeEnum.Captured)
			{
				twistInput = -mouseMotion.Relative.X * mouse_sensitivity;
				pitchInput = -mouseMotion.Relative.Y * mouse_sensitivity;
			}
		}
    }
}
