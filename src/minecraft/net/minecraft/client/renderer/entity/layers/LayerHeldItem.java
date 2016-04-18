package net.minecraft.client.renderer.entity.layers;

import net.minecraft.client.Minecraft;
import net.minecraft.client.model.ModelBiped;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.block.model.ItemCameraTransforms;
import net.minecraft.client.renderer.entity.RenderLivingBase;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumHandSide;

public class LayerHeldItem implements LayerRenderer<EntityLivingBase>
{
    protected final RenderLivingBase<?> livingEntityRenderer;

    public LayerHeldItem(RenderLivingBase<?> livingEntityRendererIn)
    {
        this.livingEntityRenderer = livingEntityRendererIn;
    }

    public void doRenderLayer(EntityLivingBase entitylivingbaseIn, float p_177141_2_, float p_177141_3_, float partialTicks, float p_177141_5_, float p_177141_6_, float p_177141_7_, float scale)
    {
        boolean flag = entitylivingbaseIn.getPrimaryHand() == EnumHandSide.RIGHT;
        ItemStack itemstack = flag ? entitylivingbaseIn.getHeldItemOffhand() : entitylivingbaseIn.getHeldItemMainhand();
        ItemStack itemstack1 = flag ? entitylivingbaseIn.getHeldItemMainhand() : entitylivingbaseIn.getHeldItemOffhand();

        if (itemstack != null || itemstack1 != null)
        {
            GlStateManager.pushMatrix();

            if (this.livingEntityRenderer.getMainModel().isChild)
            {
                float f = 0.5F;
                GlStateManager.translate(0.0F, 0.625F, 0.0F);
                GlStateManager.rotate(-20.0F, -1.0F, 0.0F, 0.0F);
                GlStateManager.scale(f, f, f);
            }

            this.func_188358_a(entitylivingbaseIn, itemstack1, ItemCameraTransforms.TransformType.THIRD_PERSON_RIGHT_HAND, EnumHandSide.RIGHT);
            this.func_188358_a(entitylivingbaseIn, itemstack, ItemCameraTransforms.TransformType.THIRD_PERSON_LEFT_HAND, EnumHandSide.LEFT);
            GlStateManager.popMatrix();
        }
    }

    private void func_188358_a(EntityLivingBase p_188358_1_, ItemStack p_188358_2_, ItemCameraTransforms.TransformType p_188358_3_, EnumHandSide p_188358_4_)
    {
        if (p_188358_2_ != null)
        {
            GlStateManager.pushMatrix();
            ((ModelBiped)this.livingEntityRenderer.getMainModel()).postRenderArm(0.0625F, p_188358_4_);

            if (p_188358_1_.isSneaking())
            {
                GlStateManager.translate(0.0F, 0.2F, 0.0F);
            }

            GlStateManager.rotate(-90.0F, 1.0F, 0.0F, 0.0F);
            GlStateManager.rotate(180.0F, 0.0F, 1.0F, 0.0F);
            boolean flag = p_188358_4_ == EnumHandSide.LEFT;
            GlStateManager.translate(flag ? -0.0625F : 0.0625F, 0.125F, -0.625F);
            Minecraft.getMinecraft().getItemRenderer().func_187462_a(p_188358_1_, p_188358_2_, p_188358_3_, flag);
            GlStateManager.popMatrix();
        }
    }

    public boolean shouldCombineTextures()
    {
        return false;
    }
}
