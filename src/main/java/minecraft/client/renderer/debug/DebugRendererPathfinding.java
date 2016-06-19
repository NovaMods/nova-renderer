package net.minecraft.client.renderer.debug;

import java.util.HashMap;
import java.util.Map;
import net.minecraft.client.Minecraft;
import net.minecraft.pathfinding.PathEntity;

public class DebugRendererPathfinding
{
    private final Minecraft field_188290_a;
    private Map<Integer, PathEntity> field_188291_b = new HashMap();
    private Map<Integer, Float> field_188292_c = new HashMap();
    private Map<Integer, Long> field_188293_d = new HashMap();
    private static float field_188294_i = 40.0F;

    public DebugRendererPathfinding(Minecraft p_i46556_1_)
    {
        this.field_188290_a = p_i46556_1_;
    }

    public void func_188289_a(int p_188289_1_, PathEntity p_188289_2_, float p_188289_3_)
    {
        this.field_188291_b.put(Integer.valueOf(p_188289_1_), p_188289_2_);
        this.field_188293_d.put(Integer.valueOf(p_188289_1_), Long.valueOf(System.currentTimeMillis()));
        this.field_188292_c.put(Integer.valueOf(p_188289_1_), Float.valueOf(p_188289_3_));
    }
}
