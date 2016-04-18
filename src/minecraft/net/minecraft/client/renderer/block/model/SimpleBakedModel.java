package net.minecraft.client.renderer.block.model;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.util.List;
import java.util.Map;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;

public class SimpleBakedModel implements IBakedModel
{
    protected final List<BakedQuad> generalQuads;
    protected final Map<EnumFacing, List<BakedQuad>> faceQuads;
    protected final boolean ambientOcclusion;
    protected final boolean gui3d;
    protected final TextureAtlasSprite texture;
    protected final ItemCameraTransforms cameraTransforms;
    protected final ItemOverrideList field_188620_g;

    public SimpleBakedModel(List<BakedQuad> p_i46535_1_, Map<EnumFacing, List<BakedQuad>> p_i46535_2_, boolean p_i46535_3_, boolean p_i46535_4_, TextureAtlasSprite p_i46535_5_, ItemCameraTransforms p_i46535_6_, ItemOverrideList p_i46535_7_)
    {
        this.generalQuads = p_i46535_1_;
        this.faceQuads = p_i46535_2_;
        this.ambientOcclusion = p_i46535_3_;
        this.gui3d = p_i46535_4_;
        this.texture = p_i46535_5_;
        this.cameraTransforms = p_i46535_6_;
        this.field_188620_g = p_i46535_7_;
    }

    public List<BakedQuad> getQuads(IBlockState state, EnumFacing side, long rand)
    {
        return side == null ? this.generalQuads : (List)this.faceQuads.get(side);
    }

    public boolean isAmbientOcclusion()
    {
        return this.ambientOcclusion;
    }

    public boolean isGui3d()
    {
        return this.gui3d;
    }

    public boolean isBuiltInRenderer()
    {
        return false;
    }

    public TextureAtlasSprite getParticleTexture()
    {
        return this.texture;
    }

    public ItemCameraTransforms getItemCameraTransforms()
    {
        return this.cameraTransforms;
    }

    public ItemOverrideList getOverrides()
    {
        return this.field_188620_g;
    }

    public static class Builder
    {
        private final List<BakedQuad> builderGeneralQuads;
        private final Map<EnumFacing, List<BakedQuad>> builderFaceQuads;
        private final ItemOverrideList field_188646_c;
        private final boolean builderAmbientOcclusion;
        private TextureAtlasSprite builderTexture;
        private boolean builderGui3d;
        private ItemCameraTransforms builderCameraTransforms;

        public Builder(ModelBlock p_i46988_1_, ItemOverrideList p_i46988_2_)
        {
            this(p_i46988_1_.isAmbientOcclusion(), p_i46988_1_.isGui3d(), p_i46988_1_.getAllTransforms(), p_i46988_2_);
        }

        public Builder(IBlockState p_i46989_1_, IBakedModel p_i46989_2_, TextureAtlasSprite p_i46989_3_, BlockPos p_i46989_4_)
        {
            this(p_i46989_2_.isAmbientOcclusion(), p_i46989_2_.isGui3d(), p_i46989_2_.getItemCameraTransforms(), p_i46989_2_.getOverrides());
            this.builderTexture = p_i46989_2_.getParticleTexture();
            long i = MathHelper.getPositionRandom(p_i46989_4_);

            for (EnumFacing enumfacing : EnumFacing.values())
            {
                this.func_188644_a(p_i46989_1_, p_i46989_2_, p_i46989_3_, enumfacing, i);
            }

            this.func_188645_a(p_i46989_1_, p_i46989_2_, p_i46989_3_, i);
        }

        private Builder(boolean p_i46990_1_, boolean p_i46990_2_, ItemCameraTransforms p_i46990_3_, ItemOverrideList p_i46990_4_)
        {
            this.builderGeneralQuads = Lists.<BakedQuad>newArrayList();
            this.builderFaceQuads = Maps.newEnumMap(EnumFacing.class);

            for (EnumFacing enumfacing : EnumFacing.values())
            {
                this.builderFaceQuads.put(enumfacing, Lists.<BakedQuad>newArrayList());
            }

            this.field_188646_c = p_i46990_4_;
            this.builderAmbientOcclusion = p_i46990_1_;
            this.builderGui3d = p_i46990_2_;
            this.builderCameraTransforms = p_i46990_3_;
        }

        private void func_188644_a(IBlockState p_188644_1_, IBakedModel p_188644_2_, TextureAtlasSprite p_188644_3_, EnumFacing p_188644_4_, long p_188644_5_)
        {
            for (BakedQuad bakedquad : p_188644_2_.getQuads(p_188644_1_, p_188644_4_, p_188644_5_))
            {
                this.addFaceQuad(p_188644_4_, new BakedQuadRetextured(bakedquad, p_188644_3_));
            }
        }

        private void func_188645_a(IBlockState p_188645_1_, IBakedModel p_188645_2_, TextureAtlasSprite p_188645_3_, long p_188645_4_)
        {
            for (BakedQuad bakedquad : p_188645_2_.getQuads(p_188645_1_, (EnumFacing)null, p_188645_4_))
            {
                this.addGeneralQuad(new BakedQuadRetextured(bakedquad, p_188645_3_));
            }
        }

        public SimpleBakedModel.Builder addFaceQuad(EnumFacing facing, BakedQuad quad)
        {
            ((List)this.builderFaceQuads.get(facing)).add(quad);
            return this;
        }

        public SimpleBakedModel.Builder addGeneralQuad(BakedQuad quad)
        {
            this.builderGeneralQuads.add(quad);
            return this;
        }

        public SimpleBakedModel.Builder setTexture(TextureAtlasSprite texture)
        {
            this.builderTexture = texture;
            return this;
        }

        public IBakedModel makeBakedModel()
        {
            if (this.builderTexture == null)
            {
                throw new RuntimeException("Missing particle!");
            }
            else
            {
                return new SimpleBakedModel(this.builderGeneralQuads, this.builderFaceQuads, this.builderAmbientOcclusion, this.builderGui3d, this.builderTexture, this.builderCameraTransforms, this.field_188646_c);
            }
        }
    }
}
