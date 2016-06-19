package net.minecraft.client.renderer.entity.layers;

import net.minecraft.client.entity.AbstractClientPlayer;
import net.minecraft.client.model.ModelElytra;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.entity.RenderPlayer;
import net.minecraft.entity.player.EnumPlayerModelParts;
import net.minecraft.init.Items;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemStack;
import net.minecraft.util.ResourceLocation;

public class LayerElytra implements LayerRenderer<AbstractClientPlayer>
{
    /** The basic Elytra texture. */
    private static final ResourceLocation TEXTURE_ELYTRA = new ResourceLocation("textures/entity/elytra.png");

    /** Instance of the player renderer. */
    private final RenderPlayer renderPlayer;

    /** The model used by the Elytra. */
    private final ModelElytra modelElytra = new ModelElytra();

    public LayerElytra(RenderPlayer p_i46546_1_)
    {
        this.renderPlayer = p_i46546_1_;
    }

    public void doRenderLayer(AbstractClientPlayer entitylivingbaseIn, float p_177141_2_, float p_177141_3_, float partialTicks, float p_177141_5_, float p_177141_6_, float p_177141_7_, float scale)
    {
        ItemStack itemstack = entitylivingbaseIn.getItemStackFromSlot(EntityEquipmentSlot.CHEST);

        if (itemstack != null && itemstack.getItem() == Items.elytra)
        {
            GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);

            if (entitylivingbaseIn.func_184833_s() && entitylivingbaseIn.getLocationElytra() != null)
            {
                this.renderPlayer.bindTexture(entitylivingbaseIn.getLocationElytra());
            }
            else if (entitylivingbaseIn.hasPlayerInfo() && entitylivingbaseIn.getLocationCape() != null && entitylivingbaseIn.isWearing(EnumPlayerModelParts.CAPE))
            {
                this.renderPlayer.bindTexture(entitylivingbaseIn.getLocationCape());
            }
            else
            {
                this.renderPlayer.bindTexture(TEXTURE_ELYTRA);
            }

            GlStateManager.pushMatrix();
            GlStateManager.translate(0.0F, 0.0F, 0.125F);
            this.modelElytra.setRotationAngles(p_177141_2_, p_177141_3_, p_177141_5_, p_177141_6_, p_177141_7_, scale, entitylivingbaseIn);
            this.modelElytra.render(entitylivingbaseIn, p_177141_2_, p_177141_3_, p_177141_5_, p_177141_6_, p_177141_7_, scale);

            if (itemstack.isItemEnchanted())
            {
                LayerArmorBase.func_188364_a(this.renderPlayer, entitylivingbaseIn, this.modelElytra, p_177141_2_, p_177141_3_, partialTicks, p_177141_5_, p_177141_6_, p_177141_7_, scale);
            }

            GlStateManager.popMatrix();
        }
    }

    public boolean shouldCombineTextures()
    {
        return false;
    }
}
