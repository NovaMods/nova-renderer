package net.minecraft.client.resources.data;

import net.minecraft.util.text.ITextComponent;

public class PackMetadataSection implements IMetadataSection
{
    private final ITextComponent packDescription;
    private final int packFormat;

    public PackMetadataSection(ITextComponent p_i1034_1_, int p_i1034_2_)
    {
        this.packDescription = p_i1034_1_;
        this.packFormat = p_i1034_2_;
    }

    public ITextComponent getPackDescription()
    {
        return this.packDescription;
    }

    public int getPackFormat()
    {
        return this.packFormat;
    }
}
