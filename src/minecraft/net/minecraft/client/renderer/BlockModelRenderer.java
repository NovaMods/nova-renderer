package net.minecraft.client.renderer;

import java.util.BitSet;
import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.color.BlockColors;
import net.minecraft.client.renderer.texture.TextureUtil;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ReportedException;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3i;
import net.minecraft.world.IBlockAccess;

public class BlockModelRenderer
{
    private final BlockColors field_187499_a;

    public BlockModelRenderer(BlockColors p_i46575_1_)
    {
        this.field_187499_a = p_i46575_1_;
    }

    public boolean renderModel(IBlockAccess blockAccessIn, IBakedModel modelIn, IBlockState blockStateIn, BlockPos blockPosIn, VertexBuffer buffer, boolean checkSides)
    {
        return this.renderModel(blockAccessIn, modelIn, blockStateIn, blockPosIn, buffer, checkSides, MathHelper.getPositionRandom(blockPosIn));
    }

    public boolean renderModel(IBlockAccess worldIn, IBakedModel modelIn, IBlockState stateIn, BlockPos posIn, VertexBuffer buffer, boolean checkSides, long rand)
    {
        boolean flag = Minecraft.isAmbientOcclusionEnabled() && stateIn.getlightValue() == 0 && modelIn.isAmbientOcclusion();

        try
        {
            return flag ? this.renderModelSmooth(worldIn, modelIn, stateIn, posIn, buffer, checkSides, rand) : this.renderModelFlat(worldIn, modelIn, stateIn, posIn, buffer, checkSides, rand);
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Tesselating block model");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Block model being tesselated");
            CrashReportCategory.addBlockInfo(crashreportcategory, posIn, stateIn);
            crashreportcategory.addCrashSection("Using AO", Boolean.valueOf(flag));
            throw new ReportedException(crashreport);
        }
    }

    public boolean renderModelSmooth(IBlockAccess worldIn, IBakedModel modelIn, IBlockState stateIn, BlockPos posIn, VertexBuffer buffer, boolean checkSides, long rand)
    {
        boolean flag = false;
        float[] afloat = new float[EnumFacing.values().length * 2];
        BitSet bitset = new BitSet(3);
        BlockModelRenderer.AmbientOcclusionFace blockmodelrenderer$ambientocclusionface = new BlockModelRenderer.AmbientOcclusionFace();

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            List<BakedQuad> list = modelIn.getQuads(stateIn, enumfacing, rand);

            if (!list.isEmpty() && (!checkSides || stateIn.shouldSideBeRendered(worldIn, posIn, enumfacing)))
            {
                this.renderQuadsSmooth(worldIn, stateIn, posIn, buffer, list, afloat, bitset, blockmodelrenderer$ambientocclusionface);
                flag = true;
            }
        }

        List<BakedQuad> list1 = modelIn.getQuads(stateIn, (EnumFacing)null, rand);

        if (!list1.isEmpty())
        {
            this.renderQuadsSmooth(worldIn, stateIn, posIn, buffer, list1, afloat, bitset, blockmodelrenderer$ambientocclusionface);
            flag = true;
        }

        return flag;
    }

    public boolean renderModelFlat(IBlockAccess worldIn, IBakedModel modelIn, IBlockState stateIn, BlockPos posIn, VertexBuffer buffer, boolean checkSides, long rand)
    {
        boolean flag = false;
        BitSet bitset = new BitSet(3);

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            List<BakedQuad> list = modelIn.getQuads(stateIn, enumfacing, rand);

            if (!list.isEmpty() && (!checkSides || stateIn.shouldSideBeRendered(worldIn, posIn, enumfacing)))
            {
                int i = stateIn.getPackedLightmapCoords(worldIn, posIn.offset(enumfacing));
                this.renderQuadsFlat(worldIn, stateIn, posIn, i, false, buffer, list, bitset);
                flag = true;
            }
        }

        List<BakedQuad> list1 = modelIn.getQuads(stateIn, (EnumFacing)null, rand);

        if (!list1.isEmpty())
        {
            this.renderQuadsFlat(worldIn, stateIn, posIn, -1, true, buffer, list1, bitset);
            flag = true;
        }

        return flag;
    }

    private void renderQuadsSmooth(IBlockAccess p_187492_1_, IBlockState p_187492_2_, BlockPos p_187492_3_, VertexBuffer p_187492_4_, List<BakedQuad> p_187492_5_, float[] p_187492_6_, BitSet p_187492_7_, BlockModelRenderer.AmbientOcclusionFace p_187492_8_)
    {
        double d0 = (double)p_187492_3_.getX();
        double d1 = (double)p_187492_3_.getY();
        double d2 = (double)p_187492_3_.getZ();
        Block block = p_187492_2_.getBlock();
        Block.EnumOffsetType block$enumoffsettype = block.getOffsetType();

        if (block$enumoffsettype != Block.EnumOffsetType.NONE)
        {
            long i = MathHelper.getPositionRandom(p_187492_3_);
            d0 += ((double)((float)(i >> 16 & 15L) / 15.0F) - 0.5D) * 0.5D;
            d2 += ((double)((float)(i >> 24 & 15L) / 15.0F) - 0.5D) * 0.5D;

            if (block$enumoffsettype == Block.EnumOffsetType.XYZ)
            {
                d1 += ((double)((float)(i >> 20 & 15L) / 15.0F) - 1.0D) * 0.2D;
            }
        }

        int l = 0;

        for (int j = p_187492_5_.size(); l < j; ++l)
        {
            BakedQuad bakedquad = (BakedQuad)p_187492_5_.get(l);
            this.fillQuadBounds(p_187492_2_, bakedquad.getVertexData(), bakedquad.getFace(), p_187492_6_, p_187492_7_);
            p_187492_8_.updateVertexBrightness(p_187492_1_, p_187492_2_, p_187492_3_, bakedquad.getFace(), p_187492_6_, p_187492_7_);
            p_187492_4_.addVertexData(bakedquad.getVertexData());
            p_187492_4_.putBrightness4(p_187492_8_.vertexBrightness[0], p_187492_8_.vertexBrightness[1], p_187492_8_.vertexBrightness[2], p_187492_8_.vertexBrightness[3]);

            if (bakedquad.hasTintIndex())
            {
                int k = this.field_187499_a.func_186724_a(p_187492_2_, p_187492_1_, p_187492_3_, bakedquad.getTintIndex());

                if (EntityRenderer.anaglyphEnable)
                {
                    k = TextureUtil.anaglyphColor(k);
                }

                float f = (float)(k >> 16 & 255) / 255.0F;
                float f1 = (float)(k >> 8 & 255) / 255.0F;
                float f2 = (float)(k & 255) / 255.0F;
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[0] * f, p_187492_8_.vertexColorMultiplier[0] * f1, p_187492_8_.vertexColorMultiplier[0] * f2, 4);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[1] * f, p_187492_8_.vertexColorMultiplier[1] * f1, p_187492_8_.vertexColorMultiplier[1] * f2, 3);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[2] * f, p_187492_8_.vertexColorMultiplier[2] * f1, p_187492_8_.vertexColorMultiplier[2] * f2, 2);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[3] * f, p_187492_8_.vertexColorMultiplier[3] * f1, p_187492_8_.vertexColorMultiplier[3] * f2, 1);
            }
            else
            {
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[0], p_187492_8_.vertexColorMultiplier[0], p_187492_8_.vertexColorMultiplier[0], 4);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[1], p_187492_8_.vertexColorMultiplier[1], p_187492_8_.vertexColorMultiplier[1], 3);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[2], p_187492_8_.vertexColorMultiplier[2], p_187492_8_.vertexColorMultiplier[2], 2);
                p_187492_4_.putColorMultiplier(p_187492_8_.vertexColorMultiplier[3], p_187492_8_.vertexColorMultiplier[3], p_187492_8_.vertexColorMultiplier[3], 1);
            }

            p_187492_4_.putPosition(d0, d1, d2);
        }
    }

    private void fillQuadBounds(IBlockState p_187494_1_, int[] p_187494_2_, EnumFacing p_187494_3_, float[] p_187494_4_, BitSet p_187494_5_)
    {
        float f = 32.0F;
        float f1 = 32.0F;
        float f2 = 32.0F;
        float f3 = -32.0F;
        float f4 = -32.0F;
        float f5 = -32.0F;

        for (int i = 0; i < 4; ++i)
        {
            float f6 = Float.intBitsToFloat(p_187494_2_[i * 7]);
            float f7 = Float.intBitsToFloat(p_187494_2_[i * 7 + 1]);
            float f8 = Float.intBitsToFloat(p_187494_2_[i * 7 + 2]);
            f = Math.min(f, f6);
            f1 = Math.min(f1, f7);
            f2 = Math.min(f2, f8);
            f3 = Math.max(f3, f6);
            f4 = Math.max(f4, f7);
            f5 = Math.max(f5, f8);
        }

        if (p_187494_4_ != null)
        {
            p_187494_4_[EnumFacing.WEST.getIndex()] = f;
            p_187494_4_[EnumFacing.EAST.getIndex()] = f3;
            p_187494_4_[EnumFacing.DOWN.getIndex()] = f1;
            p_187494_4_[EnumFacing.UP.getIndex()] = f4;
            p_187494_4_[EnumFacing.NORTH.getIndex()] = f2;
            p_187494_4_[EnumFacing.SOUTH.getIndex()] = f5;
            p_187494_4_[EnumFacing.WEST.getIndex() + EnumFacing.values().length] = 1.0F - f;
            p_187494_4_[EnumFacing.EAST.getIndex() + EnumFacing.values().length] = 1.0F - f3;
            p_187494_4_[EnumFacing.DOWN.getIndex() + EnumFacing.values().length] = 1.0F - f1;
            p_187494_4_[EnumFacing.UP.getIndex() + EnumFacing.values().length] = 1.0F - f4;
            p_187494_4_[EnumFacing.NORTH.getIndex() + EnumFacing.values().length] = 1.0F - f2;
            p_187494_4_[EnumFacing.SOUTH.getIndex() + EnumFacing.values().length] = 1.0F - f5;
        }

        float f9 = 1.0E-4F;
        float f10 = 0.9999F;

        switch (p_187494_3_)
        {
            case DOWN:
                p_187494_5_.set(1, f >= 1.0E-4F || f2 >= 1.0E-4F || f3 <= 0.9999F || f5 <= 0.9999F);
                p_187494_5_.set(0, (f1 < 1.0E-4F || p_187494_1_.isFullCube()) && f1 == f4);
                break;

            case UP:
                p_187494_5_.set(1, f >= 1.0E-4F || f2 >= 1.0E-4F || f3 <= 0.9999F || f5 <= 0.9999F);
                p_187494_5_.set(0, (f4 > 0.9999F || p_187494_1_.isFullCube()) && f1 == f4);
                break;

            case NORTH:
                p_187494_5_.set(1, f >= 1.0E-4F || f1 >= 1.0E-4F || f3 <= 0.9999F || f4 <= 0.9999F);
                p_187494_5_.set(0, (f2 < 1.0E-4F || p_187494_1_.isFullCube()) && f2 == f5);
                break;

            case SOUTH:
                p_187494_5_.set(1, f >= 1.0E-4F || f1 >= 1.0E-4F || f3 <= 0.9999F || f4 <= 0.9999F);
                p_187494_5_.set(0, (f5 > 0.9999F || p_187494_1_.isFullCube()) && f2 == f5);
                break;

            case WEST:
                p_187494_5_.set(1, f1 >= 1.0E-4F || f2 >= 1.0E-4F || f4 <= 0.9999F || f5 <= 0.9999F);
                p_187494_5_.set(0, (f < 1.0E-4F || p_187494_1_.isFullCube()) && f == f3);
                break;

            case EAST:
                p_187494_5_.set(1, f1 >= 1.0E-4F || f2 >= 1.0E-4F || f4 <= 0.9999F || f5 <= 0.9999F);
                p_187494_5_.set(0, (f3 > 0.9999F || p_187494_1_.isFullCube()) && f == f3);
        }
    }

    private void renderQuadsFlat(IBlockAccess p_187496_1_, IBlockState p_187496_2_, BlockPos p_187496_3_, int p_187496_4_, boolean p_187496_5_, VertexBuffer p_187496_6_, List<BakedQuad> p_187496_7_, BitSet p_187496_8_)
    {
        double d0 = (double)p_187496_3_.getX();
        double d1 = (double)p_187496_3_.getY();
        double d2 = (double)p_187496_3_.getZ();
        Block block = p_187496_2_.getBlock();
        Block.EnumOffsetType block$enumoffsettype = block.getOffsetType();

        if (block$enumoffsettype != Block.EnumOffsetType.NONE)
        {
            int i = p_187496_3_.getX();
            int j = p_187496_3_.getZ();
            long k = (long)(i * 3129871) ^ (long)j * 116129781L;
            k = k * k * 42317861L + k * 11L;
            d0 += ((double)((float)(k >> 16 & 15L) / 15.0F) - 0.5D) * 0.5D;
            d2 += ((double)((float)(k >> 24 & 15L) / 15.0F) - 0.5D) * 0.5D;

            if (block$enumoffsettype == Block.EnumOffsetType.XYZ)
            {
                d1 += ((double)((float)(k >> 20 & 15L) / 15.0F) - 1.0D) * 0.2D;
            }
        }

        int i1 = 0;

        for (int j1 = p_187496_7_.size(); i1 < j1; ++i1)
        {
            BakedQuad bakedquad = (BakedQuad)p_187496_7_.get(i1);

            if (p_187496_5_)
            {
                this.fillQuadBounds(p_187496_2_, bakedquad.getVertexData(), bakedquad.getFace(), (float[])null, p_187496_8_);
                p_187496_4_ = p_187496_8_.get(0) ? p_187496_2_.getPackedLightmapCoords(p_187496_1_, p_187496_3_.offset(bakedquad.getFace())) : p_187496_2_.getPackedLightmapCoords(p_187496_1_, p_187496_3_);
            }

            p_187496_6_.addVertexData(bakedquad.getVertexData());
            p_187496_6_.putBrightness4(p_187496_4_, p_187496_4_, p_187496_4_, p_187496_4_);

            if (bakedquad.hasTintIndex())
            {
                int l = this.field_187499_a.func_186724_a(p_187496_2_, p_187496_1_, p_187496_3_, bakedquad.getTintIndex());

                if (EntityRenderer.anaglyphEnable)
                {
                    l = TextureUtil.anaglyphColor(l);
                }

                float f = (float)(l >> 16 & 255) / 255.0F;
                float f1 = (float)(l >> 8 & 255) / 255.0F;
                float f2 = (float)(l & 255) / 255.0F;
                p_187496_6_.putColorMultiplier(f, f1, f2, 4);
                p_187496_6_.putColorMultiplier(f, f1, f2, 3);
                p_187496_6_.putColorMultiplier(f, f1, f2, 2);
                p_187496_6_.putColorMultiplier(f, f1, f2, 1);
            }

            p_187496_6_.putPosition(d0, d1, d2);
        }
    }

    public void renderModelBrightnessColor(IBakedModel bakedModel, float p_178262_2_, float red, float green, float blue)
    {
        this.renderModelBrightnessColor((IBlockState)null, bakedModel, p_178262_2_, red, green, blue);
    }

    public void renderModelBrightnessColor(IBlockState p_187495_1_, IBakedModel p_187495_2_, float p_187495_3_, float p_187495_4_, float p_187495_5_, float p_187495_6_)
    {
        for (EnumFacing enumfacing : EnumFacing.values())
        {
            this.renderModelBrightnessColorQuads(p_187495_3_, p_187495_4_, p_187495_5_, p_187495_6_, p_187495_2_.getQuads(p_187495_1_, enumfacing, 0L));
        }

        this.renderModelBrightnessColorQuads(p_187495_3_, p_187495_4_, p_187495_5_, p_187495_6_, p_187495_2_.getQuads(p_187495_1_, (EnumFacing)null, 0L));
    }

    public void renderModelBrightness(IBakedModel model, IBlockState p_178266_2_, float brightness, boolean p_178266_4_)
    {
        Block block = p_178266_2_.getBlock();
        GlStateManager.rotate(90.0F, 0.0F, 1.0F, 0.0F);
        int i = this.field_187499_a.func_186724_a(p_178266_2_, (IBlockAccess)null, (BlockPos)null, 0);

        if (EntityRenderer.anaglyphEnable)
        {
            i = TextureUtil.anaglyphColor(i);
        }

        float f = (float)(i >> 16 & 255) / 255.0F;
        float f1 = (float)(i >> 8 & 255) / 255.0F;
        float f2 = (float)(i & 255) / 255.0F;

        if (!p_178266_4_)
        {
            GlStateManager.color(brightness, brightness, brightness, 1.0F);
        }

        this.renderModelBrightnessColor(p_178266_2_, model, brightness, f, f1, f2);
    }

    private void renderModelBrightnessColorQuads(float brightness, float red, float green, float blue, List<BakedQuad> listQuads)
    {
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        int i = 0;

        for (int j = listQuads.size(); i < j; ++i)
        {
            BakedQuad bakedquad = (BakedQuad)listQuads.get(i);
            vertexbuffer.begin(7, DefaultVertexFormats.ITEM);
            vertexbuffer.addVertexData(bakedquad.getVertexData());

            if (bakedquad.hasTintIndex())
            {
                vertexbuffer.putColorRGB_F4(red * brightness, green * brightness, blue * brightness);
            }
            else
            {
                vertexbuffer.putColorRGB_F4(brightness, brightness, brightness);
            }

            Vec3i vec3i = bakedquad.getFace().getDirectionVec();
            vertexbuffer.putNormal((float)vec3i.getX(), (float)vec3i.getY(), (float)vec3i.getZ());
            tessellator.draw();
        }
    }

    class AmbientOcclusionFace
    {
        private final float[] vertexColorMultiplier = new float[4];
        private final int[] vertexBrightness = new int[4];

        public void updateVertexBrightness(IBlockAccess p_187491_1_, IBlockState p_187491_2_, BlockPos p_187491_3_, EnumFacing p_187491_4_, float[] p_187491_5_, BitSet p_187491_6_)
        {
            BlockPos blockpos = p_187491_6_.get(0) ? p_187491_3_.offset(p_187491_4_) : p_187491_3_;
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos = BlockPos.PooledMutableBlockPos.retain();
            BlockModelRenderer.EnumNeighborInfo blockmodelrenderer$enumneighborinfo = BlockModelRenderer.EnumNeighborInfo.getNeighbourInfo(p_187491_4_);
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos1 = BlockPos.PooledMutableBlockPos.retain(blockpos).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[0]);
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos2 = BlockPos.PooledMutableBlockPos.retain(blockpos).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[1]);
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos3 = BlockPos.PooledMutableBlockPos.retain(blockpos).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[2]);
            BlockPos.PooledMutableBlockPos blockpos$pooledmutableblockpos4 = BlockPos.PooledMutableBlockPos.retain(blockpos).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[3]);
            int i = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos$pooledmutableblockpos1);
            int j = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos$pooledmutableblockpos2);
            int k = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos$pooledmutableblockpos3);
            int l = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos$pooledmutableblockpos4);
            float f = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos1).func_185892_j();
            float f1 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos2).func_185892_j();
            float f2 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos3).func_185892_j();
            float f3 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos4).func_185892_j();
            boolean flag = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos1).func_185341_c(p_187491_4_)).isTranslucent();
            boolean flag1 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos2).func_185341_c(p_187491_4_)).isTranslucent();
            boolean flag2 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos3).func_185341_c(p_187491_4_)).isTranslucent();
            boolean flag3 = p_187491_1_.getBlockState(blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos4).func_185341_c(p_187491_4_)).isTranslucent();
            float f4;
            int i1;

            if (!flag2 && !flag)
            {
                f4 = f;
                i1 = i;
            }
            else
            {
                BlockPos blockpos1 = blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos1).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[2]);
                f4 = p_187491_1_.getBlockState(blockpos1).func_185892_j();
                i1 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos1);
            }

            float f5;
            int j1;

            if (!flag3 && !flag)
            {
                f5 = f;
                j1 = i;
            }
            else
            {
                BlockPos blockpos2 = blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos1).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[3]);
                f5 = p_187491_1_.getBlockState(blockpos2).func_185892_j();
                j1 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos2);
            }

            float f6;
            int k1;

            if (!flag2 && !flag1)
            {
                f6 = f1;
                k1 = j;
            }
            else
            {
                BlockPos blockpos3 = blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos2).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[2]);
                f6 = p_187491_1_.getBlockState(blockpos3).func_185892_j();
                k1 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos3);
            }

            float f7;
            int l1;

            if (!flag3 && !flag1)
            {
                f7 = f1;
                l1 = j;
            }
            else
            {
                BlockPos blockpos4 = blockpos$pooledmutableblockpos.set(blockpos$pooledmutableblockpos2).func_185341_c(blockmodelrenderer$enumneighborinfo.field_178276_g[3]);
                f7 = p_187491_1_.getBlockState(blockpos4).func_185892_j();
                l1 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, blockpos4);
            }

            int i3 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, p_187491_3_);

            if (p_187491_6_.get(0) || !p_187491_1_.getBlockState(p_187491_3_.offset(p_187491_4_)).isOpaqueCube())
            {
                i3 = p_187491_2_.getPackedLightmapCoords(p_187491_1_, p_187491_3_.offset(p_187491_4_));
            }

            float f8 = p_187491_6_.get(0) ? p_187491_1_.getBlockState(blockpos).func_185892_j() : p_187491_1_.getBlockState(p_187491_3_).func_185892_j();
            BlockModelRenderer.VertexTranslations blockmodelrenderer$vertextranslations = BlockModelRenderer.VertexTranslations.getVertexTranslations(p_187491_4_);
            blockpos$pooledmutableblockpos.release();
            blockpos$pooledmutableblockpos1.release();
            blockpos$pooledmutableblockpos2.release();
            blockpos$pooledmutableblockpos3.release();
            blockpos$pooledmutableblockpos4.release();

            if (p_187491_6_.get(1) && blockmodelrenderer$enumneighborinfo.field_178289_i)
            {
                float f29 = (f3 + f + f5 + f8) * 0.25F;
                float f30 = (f2 + f + f4 + f8) * 0.25F;
                float f31 = (f2 + f1 + f6 + f8) * 0.25F;
                float f32 = (f3 + f1 + f7 + f8) * 0.25F;
                float f13 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[0].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[1].field_178229_m];
                float f14 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[2].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[3].field_178229_m];
                float f15 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[4].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[5].field_178229_m];
                float f16 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[6].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178286_j[7].field_178229_m];
                float f17 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[0].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[1].field_178229_m];
                float f18 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[2].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[3].field_178229_m];
                float f19 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[4].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[5].field_178229_m];
                float f20 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[6].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178287_k[7].field_178229_m];
                float f21 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[0].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[1].field_178229_m];
                float f22 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[2].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[3].field_178229_m];
                float f23 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[4].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[5].field_178229_m];
                float f24 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[6].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178284_l[7].field_178229_m];
                float f25 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[0].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[1].field_178229_m];
                float f26 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[2].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[3].field_178229_m];
                float f27 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[4].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[5].field_178229_m];
                float f28 = p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[6].field_178229_m] * p_187491_5_[blockmodelrenderer$enumneighborinfo.field_178285_m[7].field_178229_m];
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178191_g] = f29 * f13 + f30 * f14 + f31 * f15 + f32 * f16;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178200_h] = f29 * f17 + f30 * f18 + f31 * f19 + f32 * f20;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178201_i] = f29 * f21 + f30 * f22 + f31 * f23 + f32 * f24;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178198_j] = f29 * f25 + f30 * f26 + f31 * f27 + f32 * f28;
                int i2 = this.getAoBrightness(l, i, j1, i3);
                int j2 = this.getAoBrightness(k, i, i1, i3);
                int k2 = this.getAoBrightness(k, j, k1, i3);
                int l2 = this.getAoBrightness(l, j, l1, i3);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178191_g] = this.getVertexBrightness(i2, j2, k2, l2, f13, f14, f15, f16);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178200_h] = this.getVertexBrightness(i2, j2, k2, l2, f17, f18, f19, f20);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178201_i] = this.getVertexBrightness(i2, j2, k2, l2, f21, f22, f23, f24);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178198_j] = this.getVertexBrightness(i2, j2, k2, l2, f25, f26, f27, f28);
            }
            else
            {
                float f9 = (f3 + f + f5 + f8) * 0.25F;
                float f10 = (f2 + f + f4 + f8) * 0.25F;
                float f11 = (f2 + f1 + f6 + f8) * 0.25F;
                float f12 = (f3 + f1 + f7 + f8) * 0.25F;
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178191_g] = this.getAoBrightness(l, i, j1, i3);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178200_h] = this.getAoBrightness(k, i, i1, i3);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178201_i] = this.getAoBrightness(k, j, k1, i3);
                this.vertexBrightness[blockmodelrenderer$vertextranslations.field_178198_j] = this.getAoBrightness(l, j, l1, i3);
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178191_g] = f9;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178200_h] = f10;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178201_i] = f11;
                this.vertexColorMultiplier[blockmodelrenderer$vertextranslations.field_178198_j] = f12;
            }
        }

        private int getAoBrightness(int br1, int br2, int br3, int br4)
        {
            if (br1 == 0)
            {
                br1 = br4;
            }

            if (br2 == 0)
            {
                br2 = br4;
            }

            if (br3 == 0)
            {
                br3 = br4;
            }

            return br1 + br2 + br3 + br4 >> 2 & 16711935;
        }

        private int getVertexBrightness(int p_178203_1_, int p_178203_2_, int p_178203_3_, int p_178203_4_, float p_178203_5_, float p_178203_6_, float p_178203_7_, float p_178203_8_)
        {
            int i = (int)((float)(p_178203_1_ >> 16 & 255) * p_178203_5_ + (float)(p_178203_2_ >> 16 & 255) * p_178203_6_ + (float)(p_178203_3_ >> 16 & 255) * p_178203_7_ + (float)(p_178203_4_ >> 16 & 255) * p_178203_8_) & 255;
            int j = (int)((float)(p_178203_1_ & 255) * p_178203_5_ + (float)(p_178203_2_ & 255) * p_178203_6_ + (float)(p_178203_3_ & 255) * p_178203_7_ + (float)(p_178203_4_ & 255) * p_178203_8_) & 255;
            return i << 16 | j;
        }
    }

    public static enum EnumNeighborInfo
    {
        DOWN(new EnumFacing[]{EnumFacing.WEST, EnumFacing.EAST, EnumFacing.NORTH, EnumFacing.SOUTH}, 0.5F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.SOUTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.SOUTH}),
        UP(new EnumFacing[]{EnumFacing.EAST, EnumFacing.WEST, EnumFacing.NORTH, EnumFacing.SOUTH}, 1.0F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.SOUTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.SOUTH}),
        NORTH(new EnumFacing[]{EnumFacing.UP, EnumFacing.DOWN, EnumFacing.EAST, EnumFacing.WEST}, 0.8F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_WEST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_EAST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_EAST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_WEST}),
        SOUTH(new EnumFacing[]{EnumFacing.WEST, EnumFacing.EAST, EnumFacing.DOWN, EnumFacing.UP}, 0.8F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.WEST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_WEST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.WEST, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.WEST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.EAST}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_EAST, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.EAST, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.EAST}),
        WEST(new EnumFacing[]{EnumFacing.UP, EnumFacing.DOWN, EnumFacing.NORTH, EnumFacing.SOUTH}, 0.6F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.SOUTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.SOUTH}),
        EAST(new EnumFacing[]{EnumFacing.DOWN, EnumFacing.UP, EnumFacing.NORTH, EnumFacing.SOUTH}, 0.6F, true, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.SOUTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.FLIP_DOWN, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.DOWN, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.NORTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_NORTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.NORTH}, new BlockModelRenderer.Orientation[]{BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.SOUTH, BlockModelRenderer.Orientation.FLIP_UP, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.FLIP_SOUTH, BlockModelRenderer.Orientation.UP, BlockModelRenderer.Orientation.SOUTH});

        protected final EnumFacing[] field_178276_g;
        protected final float field_178288_h;
        protected final boolean field_178289_i;
        protected final BlockModelRenderer.Orientation[] field_178286_j;
        protected final BlockModelRenderer.Orientation[] field_178287_k;
        protected final BlockModelRenderer.Orientation[] field_178284_l;
        protected final BlockModelRenderer.Orientation[] field_178285_m;
        private static final BlockModelRenderer.EnumNeighborInfo[] VALUES = new BlockModelRenderer.EnumNeighborInfo[6];

        private EnumNeighborInfo(EnumFacing[] p_i46236_3_, float p_i46236_4_, boolean p_i46236_5_, BlockModelRenderer.Orientation[] p_i46236_6_, BlockModelRenderer.Orientation[] p_i46236_7_, BlockModelRenderer.Orientation[] p_i46236_8_, BlockModelRenderer.Orientation[] p_i46236_9_)
        {
            this.field_178276_g = p_i46236_3_;
            this.field_178288_h = p_i46236_4_;
            this.field_178289_i = p_i46236_5_;
            this.field_178286_j = p_i46236_6_;
            this.field_178287_k = p_i46236_7_;
            this.field_178284_l = p_i46236_8_;
            this.field_178285_m = p_i46236_9_;
        }

        public static BlockModelRenderer.EnumNeighborInfo getNeighbourInfo(EnumFacing p_178273_0_)
        {
            return VALUES[p_178273_0_.getIndex()];
        }

        static {
            VALUES[EnumFacing.DOWN.getIndex()] = DOWN;
            VALUES[EnumFacing.UP.getIndex()] = UP;
            VALUES[EnumFacing.NORTH.getIndex()] = NORTH;
            VALUES[EnumFacing.SOUTH.getIndex()] = SOUTH;
            VALUES[EnumFacing.WEST.getIndex()] = WEST;
            VALUES[EnumFacing.EAST.getIndex()] = EAST;
        }
    }

    public static enum Orientation
    {
        DOWN(EnumFacing.DOWN, false),
        UP(EnumFacing.UP, false),
        NORTH(EnumFacing.NORTH, false),
        SOUTH(EnumFacing.SOUTH, false),
        WEST(EnumFacing.WEST, false),
        EAST(EnumFacing.EAST, false),
        FLIP_DOWN(EnumFacing.DOWN, true),
        FLIP_UP(EnumFacing.UP, true),
        FLIP_NORTH(EnumFacing.NORTH, true),
        FLIP_SOUTH(EnumFacing.SOUTH, true),
        FLIP_WEST(EnumFacing.WEST, true),
        FLIP_EAST(EnumFacing.EAST, true);

        protected final int field_178229_m;

        private Orientation(EnumFacing p_i46233_3_, boolean p_i46233_4_)
        {
            this.field_178229_m = p_i46233_3_.getIndex() + (p_i46233_4_ ? EnumFacing.values().length : 0);
        }
    }

    static enum VertexTranslations
    {
        DOWN(0, 1, 2, 3),
        UP(2, 3, 0, 1),
        NORTH(3, 0, 1, 2),
        SOUTH(0, 1, 2, 3),
        WEST(3, 0, 1, 2),
        EAST(1, 2, 3, 0);

        private final int field_178191_g;
        private final int field_178200_h;
        private final int field_178201_i;
        private final int field_178198_j;
        private static final BlockModelRenderer.VertexTranslations[] VALUES = new BlockModelRenderer.VertexTranslations[6];

        private VertexTranslations(int p_i46234_3_, int p_i46234_4_, int p_i46234_5_, int p_i46234_6_)
        {
            this.field_178191_g = p_i46234_3_;
            this.field_178200_h = p_i46234_4_;
            this.field_178201_i = p_i46234_5_;
            this.field_178198_j = p_i46234_6_;
        }

        public static BlockModelRenderer.VertexTranslations getVertexTranslations(EnumFacing p_178184_0_)
        {
            return VALUES[p_178184_0_.getIndex()];
        }

        static {
            VALUES[EnumFacing.DOWN.getIndex()] = DOWN;
            VALUES[EnumFacing.UP.getIndex()] = UP;
            VALUES[EnumFacing.NORTH.getIndex()] = NORTH;
            VALUES[EnumFacing.SOUTH.getIndex()] = SOUTH;
            VALUES[EnumFacing.WEST.getIndex()] = WEST;
            VALUES[EnumFacing.EAST.getIndex()] = EAST;
        }
    }
}
