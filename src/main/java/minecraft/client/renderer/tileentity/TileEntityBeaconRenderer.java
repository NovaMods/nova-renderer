package net.minecraft.client.renderer.tileentity;

import java.util.List;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.Tessellator;
import net.minecraft.client.renderer.VertexBuffer;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.tileentity.TileEntityBeacon;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;

public class TileEntityBeaconRenderer extends TileEntitySpecialRenderer<TileEntityBeacon>
{
    public static final ResourceLocation beaconBeam = new ResourceLocation("textures/entity/beacon_beam.png");

    public void renderTileEntityAt(TileEntityBeacon te, double x, double y, double z, float partialTicks, int destroyStage)
    {
        this.func_188206_a(x, y, z, (double)partialTicks, (double)te.shouldBeamRender(), te.getBeamSegments(), (double)te.getWorld().getTotalWorldTime());
    }

    public void func_188206_a(double p_188206_1_, double p_188206_3_, double p_188206_5_, double p_188206_7_, double p_188206_9_, List<TileEntityBeacon.BeamSegment> p_188206_11_, double p_188206_12_)
    {
        GlStateManager.alphaFunc(516, 0.1F);
        this.bindTexture(beaconBeam);

        if (p_188206_9_ > 0.0D)
        {
            GlStateManager.disableFog();
            int i = 0;

            for (int j = 0; j < p_188206_11_.size(); ++j)
            {
                TileEntityBeacon.BeamSegment tileentitybeacon$beamsegment = (TileEntityBeacon.BeamSegment)p_188206_11_.get(j);
                func_188204_a(p_188206_1_, p_188206_3_, p_188206_5_, p_188206_7_, p_188206_9_, p_188206_12_, i, tileentitybeacon$beamsegment.getHeight(), tileentitybeacon$beamsegment.getColors());
                i += tileentitybeacon$beamsegment.getHeight();
            }

            GlStateManager.enableFog();
        }
    }

    public static void func_188204_a(double p_188204_0_, double p_188204_2_, double p_188204_4_, double p_188204_6_, double p_188204_8_, double p_188204_10_, int p_188204_12_, int p_188204_13_, float[] p_188204_14_)
    {
        func_188205_a(p_188204_0_, p_188204_2_, p_188204_4_, p_188204_6_, p_188204_8_, p_188204_10_, p_188204_12_, p_188204_13_, p_188204_14_, 0.2D, 0.25D);
    }

    public static void func_188205_a(double p_188205_0_, double p_188205_2_, double p_188205_4_, double p_188205_6_, double p_188205_8_, double p_188205_10_, int p_188205_12_, int p_188205_13_, float[] p_188205_14_, double p_188205_15_, double p_188205_17_)
    {
        int i = p_188205_12_ + p_188205_13_;
        GlStateManager.glTexParameteri(3553, 10242, 10497);
        GlStateManager.glTexParameteri(3553, 10243, 10497);
        GlStateManager.disableLighting();
        GlStateManager.disableCull();
        GlStateManager.disableBlend();
        GlStateManager.depthMask(true);
        GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE, GlStateManager.SourceFactor.ONE, GlStateManager.DestFactor.ZERO);
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        double d0 = p_188205_10_ + p_188205_6_;
        double d1 = p_188205_13_ < 0 ? d0 : -d0;
        double d2 = MathHelper.func_181162_h(d1 * 0.2D - (double)MathHelper.floor_double(d1 * 0.1D));
        float f = p_188205_14_[0];
        float f1 = p_188205_14_[1];
        float f2 = p_188205_14_[2];
        double d3 = d0 * 0.025D * -1.5D;
        double d4 = 0.5D + Math.cos(d3 + 2.356194490192345D) * p_188205_15_;
        double d5 = 0.5D + Math.sin(d3 + 2.356194490192345D) * p_188205_15_;
        double d6 = 0.5D + Math.cos(d3 + (Math.PI / 4D)) * p_188205_15_;
        double d7 = 0.5D + Math.sin(d3 + (Math.PI / 4D)) * p_188205_15_;
        double d8 = 0.5D + Math.cos(d3 + 3.9269908169872414D) * p_188205_15_;
        double d9 = 0.5D + Math.sin(d3 + 3.9269908169872414D) * p_188205_15_;
        double d10 = 0.5D + Math.cos(d3 + 5.497787143782138D) * p_188205_15_;
        double d11 = 0.5D + Math.sin(d3 + 5.497787143782138D) * p_188205_15_;
        double d12 = 0.0D;
        double d13 = 1.0D;
        double d14 = -1.0D + d2;
        double d15 = (double)p_188205_13_ * p_188205_8_ * (0.5D / p_188205_15_) + d14;
        vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX_COLOR);
        vertexbuffer.pos(p_188205_0_ + d4, p_188205_2_ + (double)i, p_188205_4_ + d5).tex(1.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d4, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d5).tex(1.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d6, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d7).tex(0.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d6, p_188205_2_ + (double)i, p_188205_4_ + d7).tex(0.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d10, p_188205_2_ + (double)i, p_188205_4_ + d11).tex(1.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d10, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d11).tex(1.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d8, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d9).tex(0.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d8, p_188205_2_ + (double)i, p_188205_4_ + d9).tex(0.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d6, p_188205_2_ + (double)i, p_188205_4_ + d7).tex(1.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d6, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d7).tex(1.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d10, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d11).tex(0.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d10, p_188205_2_ + (double)i, p_188205_4_ + d11).tex(0.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d8, p_188205_2_ + (double)i, p_188205_4_ + d9).tex(1.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d8, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d9).tex(1.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d4, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d5).tex(0.0D, d14).color(f, f1, f2, 1.0F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d4, p_188205_2_ + (double)i, p_188205_4_ + d5).tex(0.0D, d15).color(f, f1, f2, 1.0F).endVertex();
        tessellator.draw();
        GlStateManager.enableBlend();
        GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA, GlStateManager.SourceFactor.ONE, GlStateManager.DestFactor.ZERO);
        GlStateManager.depthMask(false);
        d3 = 0.5D - p_188205_17_;
        d4 = 0.5D - p_188205_17_;
        d5 = 0.5D + p_188205_17_;
        d6 = 0.5D - p_188205_17_;
        d7 = 0.5D - p_188205_17_;
        d8 = 0.5D + p_188205_17_;
        d9 = 0.5D + p_188205_17_;
        d10 = 0.5D + p_188205_17_;
        d11 = 0.0D;
        d12 = 1.0D;
        d13 = -1.0D + d2;
        d14 = (double)p_188205_13_ * p_188205_8_ + d13;
        vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX_COLOR);
        vertexbuffer.pos(p_188205_0_ + d3, p_188205_2_ + (double)i, p_188205_4_ + d4).tex(1.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d3, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d4).tex(1.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d5, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d6).tex(0.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d5, p_188205_2_ + (double)i, p_188205_4_ + d6).tex(0.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d9, p_188205_2_ + (double)i, p_188205_4_ + d10).tex(1.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d9, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d10).tex(1.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d7, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d8).tex(0.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d7, p_188205_2_ + (double)i, p_188205_4_ + d8).tex(0.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d5, p_188205_2_ + (double)i, p_188205_4_ + d6).tex(1.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d5, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d6).tex(1.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d9, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d10).tex(0.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d9, p_188205_2_ + (double)i, p_188205_4_ + d10).tex(0.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d7, p_188205_2_ + (double)i, p_188205_4_ + d8).tex(1.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d7, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d8).tex(1.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d3, p_188205_2_ + (double)p_188205_12_, p_188205_4_ + d4).tex(0.0D, d13).color(f, f1, f2, 0.125F).endVertex();
        vertexbuffer.pos(p_188205_0_ + d3, p_188205_2_ + (double)i, p_188205_4_ + d4).tex(0.0D, d14).color(f, f1, f2, 0.125F).endVertex();
        tessellator.draw();
        GlStateManager.enableLighting();
        GlStateManager.enableTexture2D();
        GlStateManager.depthMask(true);
    }

    public boolean func_188185_a(TileEntityBeacon p_188185_1_)
    {
        return true;
    }
}
