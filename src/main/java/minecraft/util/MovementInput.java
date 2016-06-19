package net.minecraft.util;

public class MovementInput
{
    /**
     * The speed at which the player is strafing. Postive numbers to the left and negative to the right.
     */
    public float moveStrafe;

    /**
     * The speed at which the player is moving forward. Negative numbers will move backwards.
     */
    public float moveForward;
    public boolean field_187255_c;
    public boolean field_187256_d;
    public boolean field_187257_e;
    public boolean field_187258_f;
    public boolean jump;
    public boolean sneak;

    public void updatePlayerMoveState()
    {
    }
}
