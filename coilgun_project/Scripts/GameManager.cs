using Godot;
using System;

/// <summary>
/// Manages the game state at the highest level, including win/lose state, UI, and actions like quitting and restarting the game.
/// </summary>
public partial class GameManager : Node
{
    /// <summary>
    /// GameManager singleton instance
    /// </summary>
    public static GameManager Instance { get; private set;}

    public override void _Ready()
    {
        // Set singleton instance
        Instance = this;
    }

    public override void _Input(InputEvent @event)
    {
        // Restart the game.
        if (@event.IsActionPressed("restart"))
        {
            GetTree().ReloadCurrentScene();
        }

        // Quit the game.
		if (@event.IsActionPressed("ui_cancel"))
		{
            GetTree().Quit();
		}
    }
}