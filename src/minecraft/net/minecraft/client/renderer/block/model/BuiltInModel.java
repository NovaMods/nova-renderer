package net.minecraft.client.renderer.block.model;

import java.util.Collections;
import java.util.List;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.EnumFacing;

public class BuiltInModel implements IBakedModel
{
    private final ItemCameraTransforms cameraTransforms;
    private final ItemOverrideList field_188619_b;

    public BuiltInModel(ItemCameraTransforms p_i46537_1_, ItemOverrideList p_i46537_2_)
    {
        this.cameraTransforms = p_i46537_1_;
        this.field_188619_b = p_i46537_2_;
    }

    public List<BakedQuad> getQuads(IBlockState state, EnumFacing side, long rand)
    {
        return Collections.<BakedQuad>emptyList();
    }

    public boolean isAmbientOcclusion()
    {
        return false;
    }

    public boolean isGui3d()
    {
        return true;
    }

    public boolean isBuiltInRenderer()
    {
        return true;
    }

    public TextureAtlasSprite getParticleTexture()
    {
        return null;
    }

    public ItemCameraTransforms getItemCameraTransforms()
    {
        return this.cameraTransforms;
    }

    public ItemOverrideList getOverrides()
    {
        return this.field_188619_b;
    }
}
