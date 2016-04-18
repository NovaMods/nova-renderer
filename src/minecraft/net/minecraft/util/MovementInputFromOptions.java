package net.minecraft.util;

import net.minecraft.client.settings.GameSettings;

public class MovementInputFromOptions extends MovementInput
{
    private final GameSettings gameSettings;

    public MovementInputFromOptions(GameSettings gameSettingsIn)
    {
        this.gameSettings = gameSettingsIn;
    }

    public void updatePlayerMoveState()
    {
        this.moveStrafe = 0.0F;
        this.moveForward = 0.0F;

        if (this.gameSettings.keyBindForward.isKeyDown())
        {
            ++this.moveForward;
            this.field_187255_c = true;
        }
        else
        {
            this.field_187255_c = false;
        }

        if (this.gameSettings.keyBindBack.isKeyDown())
        {
            --this.moveForward;
            this.field_187256_d = true;
        }
        else
        {
            this.field_187256_d = false;
        }

        if (this.gameSettings.keyBindLeft.isKeyDown())
        {
            ++this.moveStrafe;
            this.field_187257_e = true;
        }
        else
        {
            this.field_187257_e = false;
        }

        if (this.gameSettings.keyBindRight.isKeyDown())
        {
            --this.moveStrafe;
            this.field_187258_f = true;
        }
        else
        {
            this.field_187258_f = false;
        }

        this.jump = this.gameSettings.keyBindJump.isKeyDown();
        this.sneak = this.gameSettings.keyBindSneak.isKeyDown();

        if (this.sneak)
        {
            this.moveStrafe = (float)((double)this.moveStrafe * 0.3D);
            this.moveForward = (float)((double)this.moveForward * 0.3D);
        }
    }
}
