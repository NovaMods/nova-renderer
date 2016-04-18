package net.minecraft.client.renderer.block.model;

import com.google.common.base.Predicate;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.EnumFacing;

public class MultipartBakedModel implements IBakedModel
{
    private final Map<Predicate<IBlockState>, IBakedModel> field_188626_f;
    protected final boolean field_188621_a;
    protected final boolean field_188622_b;
    protected final TextureAtlasSprite field_188623_c;
    protected final ItemCameraTransforms field_188624_d;
    protected final ItemOverrideList field_188625_e;

    public MultipartBakedModel(Map<Predicate<IBlockState>, IBakedModel> p_i46536_1_)
    {
        this.field_188626_f = p_i46536_1_;
        IBakedModel ibakedmodel = (IBakedModel)p_i46536_1_.values().iterator().next();
        this.field_188621_a = ibakedmodel.isAmbientOcclusion();
        this.field_188622_b = ibakedmodel.isGui3d();
        this.field_188623_c = ibakedmodel.getParticleTexture();
        this.field_188624_d = ibakedmodel.getItemCameraTransforms();
        this.field_188625_e = ibakedmodel.getOverrides();
    }

    public List<BakedQuad> getQuads(IBlockState state, EnumFacing side, long rand)
    {
        List<BakedQuad> list = Lists.<BakedQuad>newArrayList();

        if (state != null)
        {
            for (Entry<Predicate<IBlockState>, IBakedModel> entry : this.field_188626_f.entrySet())
            {
                if (((Predicate)entry.getKey()).apply(state))
                {
                    list.addAll(((IBakedModel)entry.getValue()).getQuads(state, side, rand++));
                }
            }
        }

        return list;
    }

    public boolean isAmbientOcclusion()
    {
        return this.field_188621_a;
    }

    public boolean isGui3d()
    {
        return this.field_188622_b;
    }

    public boolean isBuiltInRenderer()
    {
        return false;
    }

    public TextureAtlasSprite getParticleTexture()
    {
        return this.field_188623_c;
    }

    public ItemCameraTransforms getItemCameraTransforms()
    {
        return this.field_188624_d;
    }

    public ItemOverrideList getOverrides()
    {
        return this.field_188625_e;
    }

    public static class Builder
    {
        private Map<Predicate<IBlockState>, IBakedModel> field_188649_a = Maps.<Predicate<IBlockState>, IBakedModel>newLinkedHashMap();

        public void func_188648_a(Predicate<IBlockState> p_188648_1_, IBakedModel p_188648_2_)
        {
            this.field_188649_a.put(p_188648_1_, p_188648_2_);
        }

        public IBakedModel func_188647_a()
        {
            return new MultipartBakedModel(this.field_188649_a);
        }
    }
}
