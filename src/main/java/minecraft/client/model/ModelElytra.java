package net.minecraft.client.model;

import net.minecraft.client.entity.AbstractClientPlayer;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.util.math.Vec3d;

public class ModelElytra extends ModelBase
{
    private ModelRenderer field_187060_a;
    private ModelRenderer field_187061_b = new ModelRenderer(this, 22, 0);

    public ModelElytra()
    {
        this.field_187061_b.addBox(-10.0F, 0.0F, 0.0F, 10, 20, 2, 1.0F);
        this.field_187060_a = new ModelRenderer(this, 22, 0);
        this.field_187060_a.mirror = true;
        this.field_187060_a.addBox(0.0F, 0.0F, 0.0F, 10, 20, 2, 1.0F);
    }

    /**
     * Sets the models various rotation angles then renders the model.
     */
    public void render(Entity entityIn, float p_78088_2_, float p_78088_3_, float p_78088_4_, float p_78088_5_, float p_78088_6_, float scale)
    {
        GlStateManager.disableRescaleNormal();
        GlStateManager.disableCull();
        this.field_187061_b.render(scale);
        this.field_187060_a.render(scale);
    }

    /**
     * Sets the model's various rotation angles. For bipeds, par1 and par2 are used for animating the movement of arms
     * and legs, where par1 represents the time(so that arms and legs swing back and forth) and par2 represents how
     * "far" arms and legs can swing at most.
     */
    public void setRotationAngles(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch, float scaleFactor, Entity entityIn)
    {
        super.setRotationAngles(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scaleFactor, entityIn);
        float f = 0.2617994F;
        float f1 = -0.2617994F;
        float f2 = 0.0F;
        float f3 = 0.0F;

        if (entityIn instanceof EntityLivingBase && ((EntityLivingBase)entityIn).func_184613_cA())
        {
            float f4 = 1.0F;

            if (entityIn.motionY < 0.0D)
            {
                Vec3d vec3d = (new Vec3d(entityIn.motionX, entityIn.motionY, entityIn.motionZ)).normalize();
                f4 = 1.0F - (float)Math.pow(-vec3d.yCoord, 1.5D);
            }

            f = f4 * 0.34906584F + (1.0F - f4) * f;
            f1 = f4 * -((float)Math.PI / 2F) + (1.0F - f4) * f1;
        }
        else if (entityIn.isSneaking())
        {
            f = ((float)Math.PI * 2F / 9F);
            f1 = -((float)Math.PI / 4F);
            f2 = 3.0F;
            f3 = 0.08726646F;
        }

        this.field_187061_b.rotationPointX = 5.0F;
        this.field_187061_b.rotationPointY = f2;

        if (entityIn instanceof AbstractClientPlayer)
        {
            AbstractClientPlayer abstractclientplayer = (AbstractClientPlayer)entityIn;
            abstractclientplayer.field_184835_a = (float)((double)abstractclientplayer.field_184835_a + (double)(f - abstractclientplayer.field_184835_a) * 0.1D);
            abstractclientplayer.field_184836_b = (float)((double)abstractclientplayer.field_184836_b + (double)(f3 - abstractclientplayer.field_184836_b) * 0.1D);
            abstractclientplayer.field_184837_c = (float)((double)abstractclientplayer.field_184837_c + (double)(f1 - abstractclientplayer.field_184837_c) * 0.1D);
            this.field_187061_b.rotateAngleX = abstractclientplayer.field_184835_a;
            this.field_187061_b.rotateAngleY = abstractclientplayer.field_184836_b;
            this.field_187061_b.rotateAngleZ = abstractclientplayer.field_184837_c;
        }
        else
        {
            this.field_187061_b.rotateAngleX = f;
            this.field_187061_b.rotateAngleZ = f1;
            this.field_187061_b.rotateAngleY = f3;
        }

        this.field_187060_a.rotationPointX = -this.field_187061_b.rotationPointX;
        this.field_187060_a.rotateAngleY = -this.field_187061_b.rotateAngleY;
        this.field_187060_a.rotationPointY = this.field_187061_b.rotationPointY;
        this.field_187060_a.rotateAngleX = this.field_187061_b.rotateAngleX;
        this.field_187060_a.rotateAngleZ = -this.field_187061_b.rotateAngleZ;
    }

    /**
     * Used for easily adding entity-dependent animations. The second and third float params here are the same second
     * and third as in the setRotationAngles method.
     */
    public void setLivingAnimations(EntityLivingBase entitylivingbaseIn, float p_78086_2_, float p_78086_3_, float partialTickTime)
    {
        super.setLivingAnimations(entitylivingbaseIn, p_78086_2_, p_78086_3_, partialTickTime);
    }
}
