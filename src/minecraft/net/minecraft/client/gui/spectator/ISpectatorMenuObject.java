package net.minecraft.client.gui.spectator;

import net.minecraft.util.text.ITextComponent;

public interface ISpectatorMenuObject
{
    void func_178661_a(SpectatorMenu menu);

    ITextComponent getSpectatorName();

    void func_178663_a(float p_178663_1_, int alpha);

    boolean func_178662_A_();
}
