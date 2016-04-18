package net.minecraft.client.renderer.debug;

import net.minecraft.client.Minecraft;

public class DebugRenderer
{
    public final DebugRendererPathfinding field_188286_a;
    public final DebugRendererWater field_188287_b;

    public DebugRenderer(Minecraft p_i46557_1_)
    {
        this.field_188286_a = new DebugRendererPathfinding(p_i46557_1_);
        this.field_188287_b = new DebugRendererWater(p_i46557_1_);
    }
}
