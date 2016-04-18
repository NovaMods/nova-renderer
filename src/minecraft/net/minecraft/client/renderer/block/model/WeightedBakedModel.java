package net.minecraft.client.renderer.block.model;

import com.google.common.collect.ComparisonChain;
import com.google.common.collect.Lists;
import java.util.Collections;
import java.util.List;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.WeightedRandom;

public class WeightedBakedModel implements IBakedModel
{
    private final int totalWeight;
    private final List<WeightedBakedModel.WeightedModel> models;
    private final IBakedModel baseModel;

    public WeightedBakedModel(List<WeightedBakedModel.WeightedModel> p_i46073_1_)
    {
        this.models = p_i46073_1_;
        this.totalWeight = WeightedRandom.getTotalWeight(p_i46073_1_);
        this.baseModel = ((WeightedBakedModel.WeightedModel)p_i46073_1_.get(0)).field_185281_b;
    }

    private IBakedModel func_188627_a(long p_188627_1_)
    {
        return ((WeightedBakedModel.WeightedModel)WeightedRandom.getRandomItem(this.models, Math.abs((int)p_188627_1_ >> 16) % this.totalWeight)).field_185281_b;
    }

    public List<BakedQuad> getQuads(IBlockState state, EnumFacing side, long rand)
    {
        return this.func_188627_a(rand).getQuads(state, side, rand);
    }

    public boolean isAmbientOcclusion()
    {
        return this.baseModel.isAmbientOcclusion();
    }

    public boolean isGui3d()
    {
        return this.baseModel.isGui3d();
    }

    public boolean isBuiltInRenderer()
    {
        return this.baseModel.isBuiltInRenderer();
    }

    public TextureAtlasSprite getParticleTexture()
    {
        return this.baseModel.getParticleTexture();
    }

    public ItemCameraTransforms getItemCameraTransforms()
    {
        return this.baseModel.getItemCameraTransforms();
    }

    public ItemOverrideList getOverrides()
    {
        return this.baseModel.getOverrides();
    }

    public static class Builder
    {
        private List<WeightedBakedModel.WeightedModel> listItems = Lists.<WeightedBakedModel.WeightedModel>newArrayList();

        public WeightedBakedModel.Builder add(IBakedModel p_177677_1_, int p_177677_2_)
        {
            this.listItems.add(new WeightedBakedModel.WeightedModel(p_177677_1_, p_177677_2_));
            return this;
        }

        public WeightedBakedModel build()
        {
            Collections.sort(this.listItems);
            return new WeightedBakedModel(this.listItems);
        }

        public IBakedModel first()
        {
            return ((WeightedBakedModel.WeightedModel)this.listItems.get(0)).field_185281_b;
        }
    }

    static class WeightedModel extends WeightedRandom.Item implements Comparable<WeightedBakedModel.WeightedModel>
    {
        protected final IBakedModel field_185281_b;

        public WeightedModel(IBakedModel p_i46763_1_, int p_i46763_2_)
        {
            super(p_i46763_2_);
            this.field_185281_b = p_i46763_1_;
        }

        public int compareTo(WeightedBakedModel.WeightedModel p_compareTo_1_)
        {
            return ComparisonChain.start().compare(p_compareTo_1_.itemWeight, this.itemWeight).result();
        }

        public String toString()
        {
            return "MyWeighedRandomItem{weight=" + this.itemWeight + ", model=" + this.field_185281_b + '}';
        }
    }
}
