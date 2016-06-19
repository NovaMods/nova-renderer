package net.minecraft.client.renderer.block.model;

import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.util.EnumFacing;

public class BakedQuad
{
    /**
     * Joined 4 vertex records, each has 7 fields (x, y, z, shadeColor, u, v, <unused>), see
     * FaceBakery.storeVertexData()
     */
    protected final int[] vertexData;
    protected final int tintIndex;
    protected final EnumFacing face;
    protected final TextureAtlasSprite sprite;

    public BakedQuad(int[] p_i46574_1_, int p_i46574_2_, EnumFacing p_i46574_3_, TextureAtlasSprite p_i46574_4_)
    {
        this.vertexData = p_i46574_1_;
        this.tintIndex = p_i46574_2_;
        this.face = p_i46574_3_;
        this.sprite = p_i46574_4_;
    }

    public TextureAtlasSprite getSprite()
    {
        return this.sprite;
    }

    public int[] getVertexData()
    {
        return this.vertexData;
    }

    public boolean hasTintIndex()
    {
        return this.tintIndex != -1;
    }

    public int getTintIndex()
    {
        return this.tintIndex;
    }

    public EnumFacing getFace()
    {
        return this.face;
    }
}
