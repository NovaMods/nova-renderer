package net.minecraft.client.resources.data;

public class TextureMetadataSection implements IMetadataSection
{
    private final boolean textureBlur;
    private final boolean textureClamp;

    public TextureMetadataSection(boolean p_i46538_1_, boolean p_i46538_2_)
    {
        this.textureBlur = p_i46538_1_;
        this.textureClamp = p_i46538_2_;
    }

    public boolean getTextureBlur()
    {
        return this.textureBlur;
    }

    public boolean getTextureClamp()
    {
        return this.textureClamp;
    }
}
