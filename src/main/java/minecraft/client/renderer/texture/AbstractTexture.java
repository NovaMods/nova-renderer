package net.minecraft.client.renderer.texture;

import net.minecraft.client.renderer.GlStateManager;

public abstract class AbstractTexture implements ITextureObject
{
    protected int glTextureId = -1;
    protected boolean blur;
    protected boolean mipmap;
    protected boolean blurLast;
    protected boolean mipmapLast;

    public void setBlurMipmapDirect(boolean p_174937_1_, boolean p_174937_2_)
    {
        this.blur = p_174937_1_;
        this.mipmap = p_174937_2_;
        int i = -1;
        int j = -1;

        if (p_174937_1_)
        {
            i = p_174937_2_ ? 9987 : 9729;
            j = 9729;
        }
        else
        {
            i = p_174937_2_ ? 9986 : 9728;
            j = 9728;
        }

        GlStateManager.glTexParameteri(3553, 10241, i);
        GlStateManager.glTexParameteri(3553, 10240, j);
    }

    public void setBlurMipmap(boolean p_174936_1_, boolean p_174936_2_)
    {
        this.blurLast = this.blur;
        this.mipmapLast = this.mipmap;
        this.setBlurMipmapDirect(p_174936_1_, p_174936_2_);
    }

    public void restoreLastBlurMipmap()
    {
        this.setBlurMipmapDirect(this.blurLast, this.mipmapLast);
    }

    public int getGlTextureId()
    {
        if (this.glTextureId == -1)
        {
            this.glTextureId = TextureUtil.glGenTextures();
        }

        return this.glTextureId;
    }

    public void deleteGlTexture()
    {
        if (this.glTextureId != -1)
        {
            TextureUtil.deleteTexture(this.glTextureId);
            this.glTextureId = -1;
        }
    }
}
