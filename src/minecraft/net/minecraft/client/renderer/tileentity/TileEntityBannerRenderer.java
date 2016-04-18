package net.minecraft.client.renderer.tileentity;

import net.minecraft.client.model.ModelBanner;
import net.minecraft.client.renderer.BannerTextures;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.init.Blocks;
import net.minecraft.tileentity.TileEntityBanner;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;

public class TileEntityBannerRenderer extends TileEntitySpecialRenderer<TileEntityBanner>
{
    private ModelBanner bannerModel = new ModelBanner();

    public void renderTileEntityAt(TileEntityBanner te, double x, double y, double z, float partialTicks, int destroyStage)
    {
        boolean flag = te.getWorld() != null;
        boolean flag1 = !flag || te.getBlockType() == Blocks.standing_banner;
        int i = flag ? te.getBlockMetadata() : 0;
        long j = flag ? te.getWorld().getTotalWorldTime() : 0L;
        GlStateManager.pushMatrix();
        float f = 0.6666667F;

        if (flag1)
        {
            GlStateManager.translate((float)x + 0.5F, (float)y + 0.75F * f, (float)z + 0.5F);
            float f1 = (float)(i * 360) / 16.0F;
            GlStateManager.rotate(-f1, 0.0F, 1.0F, 0.0F);
            this.bannerModel.bannerStand.showModel = true;
        }
        else
        {
            float f2 = 0.0F;

            if (i == 2)
            {
                f2 = 180.0F;
            }

            if (i == 4)
            {
                f2 = 90.0F;
            }

            if (i == 5)
            {
                f2 = -90.0F;
            }

            GlStateManager.translate((float)x + 0.5F, (float)y - 0.25F * f, (float)z + 0.5F);
            GlStateManager.rotate(-f2, 0.0F, 1.0F, 0.0F);
            GlStateManager.translate(0.0F, -0.3125F, -0.4375F);
            this.bannerModel.bannerStand.showModel = false;
        }

        BlockPos blockpos = te.getPos();
        float f3 = (float)(blockpos.getX() * 7 + blockpos.getY() * 9 + blockpos.getZ() * 13) + (float)j + partialTicks;
        this.bannerModel.bannerSlate.rotateAngleX = (-0.0125F + 0.01F * MathHelper.cos(f3 * (float)Math.PI * 0.02F)) * (float)Math.PI;
        GlStateManager.enableRescaleNormal();
        ResourceLocation resourcelocation = this.func_178463_a(te);

        if (resourcelocation != null)
        {
            this.bindTexture(resourcelocation);
            GlStateManager.pushMatrix();
            GlStateManager.scale(f, -f, -f);
            this.bannerModel.renderBanner();
            GlStateManager.popMatrix();
        }

        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        GlStateManager.popMatrix();
    }

    private ResourceLocation func_178463_a(TileEntityBanner bannerObj)
    {
        return BannerTextures.DESIGNS.func_187478_a(bannerObj.getPatternResourceLocation(), bannerObj.getPatternList(), bannerObj.getColorList());
    }
}
