package net.minecraft.client.model;

import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.entity.Entity;

public class ModelDragonHead extends ModelBase
{
    private ModelRenderer field_187070_a;
    private ModelRenderer field_187071_b;

    public ModelDragonHead(float p_i46588_1_)
    {
        this.textureWidth = 256;
        this.textureHeight = 256;
        this.setTextureOffset("body.body", 0, 0);
        this.setTextureOffset("wing.skin", -56, 88);
        this.setTextureOffset("wingtip.skin", -56, 144);
        this.setTextureOffset("rearleg.main", 0, 0);
        this.setTextureOffset("rearfoot.main", 112, 0);
        this.setTextureOffset("rearlegtip.main", 196, 0);
        this.setTextureOffset("head.upperhead", 112, 30);
        this.setTextureOffset("wing.bone", 112, 88);
        this.setTextureOffset("head.upperlip", 176, 44);
        this.setTextureOffset("jaw.jaw", 176, 65);
        this.setTextureOffset("frontleg.main", 112, 104);
        this.setTextureOffset("wingtip.bone", 112, 136);
        this.setTextureOffset("frontfoot.main", 144, 104);
        this.setTextureOffset("neck.box", 192, 104);
        this.setTextureOffset("frontlegtip.main", 226, 138);
        this.setTextureOffset("body.scale", 220, 53);
        this.setTextureOffset("head.scale", 0, 0);
        this.setTextureOffset("neck.scale", 48, 0);
        this.setTextureOffset("head.nostril", 112, 0);
        float f = -16.0F;
        this.field_187070_a = new ModelRenderer(this, "head");
        this.field_187070_a.addBox("upperlip", -6.0F, -1.0F, -8.0F + f, 12, 5, 16);
        this.field_187070_a.addBox("upperhead", -8.0F, -8.0F, 6.0F + f, 16, 16, 16);
        this.field_187070_a.mirror = true;
        this.field_187070_a.addBox("scale", -5.0F, -12.0F, 12.0F + f, 2, 4, 6);
        this.field_187070_a.addBox("nostril", -5.0F, -3.0F, -6.0F + f, 2, 2, 4);
        this.field_187070_a.mirror = false;
        this.field_187070_a.addBox("scale", 3.0F, -12.0F, 12.0F + f, 2, 4, 6);
        this.field_187070_a.addBox("nostril", 3.0F, -3.0F, -6.0F + f, 2, 2, 4);
        this.field_187071_b = new ModelRenderer(this, "jaw");
        this.field_187071_b.setRotationPoint(0.0F, 4.0F, 8.0F + f);
        this.field_187071_b.addBox("jaw", -6.0F, 0.0F, -16.0F, 12, 4, 16);
        this.field_187070_a.addChild(this.field_187071_b);
    }

    /**
     * Sets the models various rotation angles then renders the model.
     */
    public void render(Entity entityIn, float p_78088_2_, float p_78088_3_, float p_78088_4_, float p_78088_5_, float p_78088_6_, float scale)
    {
        this.field_187071_b.rotateAngleX = (float)(Math.sin((double)(p_78088_2_ * (float)Math.PI * 0.2F)) + 1.0D) * 0.2F;
        this.field_187070_a.rotateAngleY = p_78088_5_ * 0.017453292F;
        this.field_187070_a.rotateAngleX = p_78088_6_ * 0.017453292F;
        GlStateManager.translate(0.0F, -0.374375F, 0.0F);
        GlStateManager.scale(0.75F, 0.75F, 0.75F);
        this.field_187070_a.render(scale);
    }
}
