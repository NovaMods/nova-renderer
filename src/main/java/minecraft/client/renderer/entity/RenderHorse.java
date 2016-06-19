package net.minecraft.client.renderer.entity;

import com.google.common.collect.Maps;
import java.util.Map;
import net.minecraft.client.Minecraft;
import net.minecraft.client.model.ModelHorse;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.texture.LayeredTexture;
import net.minecraft.entity.passive.EntityHorse;
import net.minecraft.entity.passive.HorseArmorType;
import net.minecraft.util.ResourceLocation;

public class RenderHorse extends RenderLiving<EntityHorse>
{
    private static final Map<String, ResourceLocation> field_110852_a = Maps.<String, ResourceLocation>newHashMap();

    public RenderHorse(RenderManager rendermanagerIn, ModelHorse model, float shadowSizeIn)
    {
        super(rendermanagerIn, model, shadowSizeIn);
    }

    /**
     * Allows the render to do any OpenGL state modifications necessary before the model is rendered. Args:
     * entityLiving, partialTickTime
     */
    protected void preRenderCallback(EntityHorse entitylivingbaseIn, float partialTickTime)
    {
        float f = 1.0F;
        HorseArmorType horsearmortype = entitylivingbaseIn.getType();

        if (horsearmortype == HorseArmorType.DONKEY)
        {
            f *= 0.87F;
        }
        else if (horsearmortype == HorseArmorType.MULE)
        {
            f *= 0.92F;
        }

        GlStateManager.scale(f, f, f);
        super.preRenderCallback(entitylivingbaseIn, partialTickTime);
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(EntityHorse entity)
    {
        return !entity.func_110239_cn() ? entity.getType().func_188592_e() : this.func_188328_b(entity);
    }

    private ResourceLocation func_188328_b(EntityHorse p_188328_1_)
    {
        String s = p_188328_1_.getHorseTexture();

        if (!p_188328_1_.func_175507_cI())
        {
            return null;
        }
        else
        {
            ResourceLocation resourcelocation = (ResourceLocation)field_110852_a.get(s);

            if (resourcelocation == null)
            {
                resourcelocation = new ResourceLocation(s);
                Minecraft.getMinecraft().getTextureManager().loadTexture(resourcelocation, new LayeredTexture(p_188328_1_.getVariantTexturePaths()));
                field_110852_a.put(s, resourcelocation);
            }

            return resourcelocation;
        }
    }
}
