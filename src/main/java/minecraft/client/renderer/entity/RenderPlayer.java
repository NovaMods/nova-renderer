package net.minecraft.client.renderer.entity;

import net.minecraft.client.entity.AbstractClientPlayer;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.model.ModelBiped;
import net.minecraft.client.model.ModelPlayer;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.entity.layers.LayerArrow;
import net.minecraft.client.renderer.entity.layers.LayerBipedArmor;
import net.minecraft.client.renderer.entity.layers.LayerCape;
import net.minecraft.client.renderer.entity.layers.LayerCustomHead;
import net.minecraft.client.renderer.entity.layers.LayerDeadmau5Head;
import net.minecraft.client.renderer.entity.layers.LayerElytra;
import net.minecraft.client.renderer.entity.layers.LayerHeldItem;
import net.minecraft.entity.player.EnumPlayerModelParts;
import net.minecraft.item.EnumAction;
import net.minecraft.item.ItemStack;
import net.minecraft.scoreboard.Score;
import net.minecraft.scoreboard.ScoreObjective;
import net.minecraft.scoreboard.Scoreboard;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;

public class RenderPlayer extends RenderLivingBase<AbstractClientPlayer>
{
    /** this field is used to indicate the 3-pixel wide arms */
    private boolean smallArms;

    public RenderPlayer(RenderManager renderManager)
    {
        this(renderManager, false);
    }

    public RenderPlayer(RenderManager renderManager, boolean useSmallArms)
    {
        super(renderManager, new ModelPlayer(0.0F, useSmallArms), 0.5F);
        this.smallArms = useSmallArms;
        this.addLayer(new LayerBipedArmor(this));
        this.addLayer(new LayerHeldItem(this));
        this.addLayer(new LayerArrow(this));
        this.addLayer(new LayerDeadmau5Head(this));
        this.addLayer(new LayerCape(this));
        this.addLayer(new LayerCustomHead(this.getMainModel().bipedHead));
        this.addLayer(new LayerElytra(this));
    }

    public ModelPlayer getMainModel()
    {
        return (ModelPlayer)super.getMainModel();
    }

    /**
     * Renders the desired {@code T} type Entity.
     */
    public void doRender(AbstractClientPlayer entity, double x, double y, double z, float entityYaw, float partialTicks)
    {
        if (!entity.isUser() || this.renderManager.livingPlayer == entity)
        {
            double d0 = y;

            if (entity.isSneaking() && !(entity instanceof EntityPlayerSP))
            {
                d0 = y - 0.125D;
            }

            this.setModelVisibilities(entity);
            GlStateManager.enableBlendProfile(GlStateManager.Profile.PLAYER_SKIN);
            super.doRender(entity, x, d0, z, entityYaw, partialTicks);
            GlStateManager.disableBlendProfile(GlStateManager.Profile.PLAYER_SKIN);
        }
    }

    private void setModelVisibilities(AbstractClientPlayer clientPlayer)
    {
        ModelPlayer modelplayer = this.getMainModel();

        if (clientPlayer.isSpectator())
        {
            modelplayer.setInvisible(false);
            modelplayer.bipedHead.showModel = true;
            modelplayer.bipedHeadwear.showModel = true;
        }
        else
        {
            ItemStack itemstack = clientPlayer.getHeldItemMainhand();
            ItemStack itemstack1 = clientPlayer.getHeldItemOffhand();
            modelplayer.setInvisible(true);
            modelplayer.bipedHeadwear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.HAT);
            modelplayer.bipedBodyWear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.JACKET);
            modelplayer.bipedLeftLegwear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.LEFT_PANTS_LEG);
            modelplayer.bipedRightLegwear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.RIGHT_PANTS_LEG);
            modelplayer.bipedLeftArmwear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.LEFT_SLEEVE);
            modelplayer.bipedRightArmwear.showModel = clientPlayer.isWearing(EnumPlayerModelParts.RIGHT_SLEEVE);
            modelplayer.isSneak = clientPlayer.isSneaking();
            ModelBiped.ArmPose modelbiped$armpose = ModelBiped.ArmPose.EMPTY;
            ModelBiped.ArmPose modelbiped$armpose1 = ModelBiped.ArmPose.EMPTY;

            if (itemstack != null)
            {
                modelbiped$armpose = ModelBiped.ArmPose.ITEM;

                if (clientPlayer.getItemInUseCount() > 0)
                {
                    EnumAction enumaction = itemstack.getItemUseAction();

                    if (enumaction == EnumAction.BLOCK)
                    {
                        modelbiped$armpose = ModelBiped.ArmPose.BLOCK;
                    }
                    else if (enumaction == EnumAction.BOW)
                    {
                        modelbiped$armpose = ModelBiped.ArmPose.BOW_AND_ARROW;
                    }
                }
            }

            if (itemstack1 != null)
            {
                modelbiped$armpose1 = ModelBiped.ArmPose.ITEM;

                if (clientPlayer.getItemInUseCount() > 0)
                {
                    EnumAction enumaction1 = itemstack1.getItemUseAction();

                    if (enumaction1 == EnumAction.BLOCK)
                    {
                        modelbiped$armpose1 = ModelBiped.ArmPose.BLOCK;
                    }
                }
            }

            if (clientPlayer.getPrimaryHand() == EnumHandSide.RIGHT)
            {
                modelplayer.rightArmPose = modelbiped$armpose;
                modelplayer.leftArmPose = modelbiped$armpose1;
            }
            else
            {
                modelplayer.rightArmPose = modelbiped$armpose1;
                modelplayer.leftArmPose = modelbiped$armpose;
            }
        }
    }

    /**
     * Returns the location of an entity's texture. Doesn't seem to be called unless you call Render.bindEntityTexture.
     */
    protected ResourceLocation getEntityTexture(AbstractClientPlayer entity)
    {
        return entity.getLocationSkin();
    }

    public void transformHeldFull3DItemLayer()
    {
        GlStateManager.translate(0.0F, 0.1875F, 0.0F);
    }

    /**
     * Allows the render to do any OpenGL state modifications necessary before the model is rendered. Args:
     * entityLiving, partialTickTime
     */
    protected void preRenderCallback(AbstractClientPlayer entitylivingbaseIn, float partialTickTime)
    {
        float f = 0.9375F;
        GlStateManager.scale(f, f, f);
    }

    protected void func_188296_a(AbstractClientPlayer p_188296_1_, double p_188296_2_, double p_188296_4_, double p_188296_6_, String p_188296_8_, double p_188296_9_)
    {
        if (p_188296_9_ < 100.0D)
        {
            Scoreboard scoreboard = p_188296_1_.getWorldScoreboard();
            ScoreObjective scoreobjective = scoreboard.getObjectiveInDisplaySlot(2);

            if (scoreobjective != null)
            {
                Score score = scoreboard.getValueFromObjective(p_188296_1_.getName(), scoreobjective);
                this.renderLivingLabel(p_188296_1_, score.getScorePoints() + " " + scoreobjective.getDisplayName(), p_188296_2_, p_188296_4_, p_188296_6_, 64);
                p_188296_4_ += (double)((float)this.getFontRendererFromRenderManager().FONT_HEIGHT * 1.15F * 0.025F);
            }
        }

        super.func_188296_a(p_188296_1_, p_188296_2_, p_188296_4_, p_188296_6_, p_188296_8_, p_188296_9_);
    }

    public void renderRightArm(AbstractClientPlayer clientPlayer)
    {
        float f = 1.0F;
        GlStateManager.color(f, f, f);
        float f1 = 0.0625F;
        ModelPlayer modelplayer = this.getMainModel();
        this.setModelVisibilities(clientPlayer);
        GlStateManager.enableBlend();
        modelplayer.swingProgress = 0.0F;
        modelplayer.isSneak = false;
        modelplayer.setRotationAngles(0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0625F, clientPlayer);
        modelplayer.bipedRightArm.rotateAngleX = 0.0F;
        modelplayer.bipedRightArm.render(0.0625F);
        modelplayer.bipedRightArmwear.rotateAngleX = 0.0F;
        modelplayer.bipedRightArmwear.render(0.0625F);
        GlStateManager.disableBlend();
    }

    public void renderLeftArm(AbstractClientPlayer clientPlayer)
    {
        float f = 1.0F;
        GlStateManager.color(f, f, f);
        float f1 = 0.0625F;
        ModelPlayer modelplayer = this.getMainModel();
        this.setModelVisibilities(clientPlayer);
        GlStateManager.enableBlend();
        modelplayer.isSneak = false;
        modelplayer.swingProgress = 0.0F;
        modelplayer.setRotationAngles(0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0625F, clientPlayer);
        modelplayer.bipedLeftArm.rotateAngleX = 0.0F;
        modelplayer.bipedLeftArm.render(0.0625F);
        modelplayer.bipedLeftArmwear.rotateAngleX = 0.0F;
        modelplayer.bipedLeftArmwear.render(0.0625F);
        GlStateManager.disableBlend();
    }

    /**
     * Sets a simple glTranslate on a LivingEntity.
     */
    protected void renderLivingAt(AbstractClientPlayer entityLivingBaseIn, double x, double y, double z)
    {
        if (entityLivingBaseIn.isEntityAlive() && entityLivingBaseIn.isPlayerSleeping())
        {
            super.renderLivingAt(entityLivingBaseIn, x + (double)entityLivingBaseIn.renderOffsetX, y + (double)entityLivingBaseIn.renderOffsetY, z + (double)entityLivingBaseIn.renderOffsetZ);
        }
        else
        {
            super.renderLivingAt(entityLivingBaseIn, x, y, z);
        }
    }

    protected void rotateCorpse(AbstractClientPlayer bat, float p_77043_2_, float p_77043_3_, float partialTicks)
    {
        if (bat.isEntityAlive() && bat.isPlayerSleeping())
        {
            GlStateManager.rotate(bat.getBedOrientationInDegrees(), 0.0F, 1.0F, 0.0F);
            GlStateManager.rotate(this.getDeathMaxRotation(bat), 0.0F, 0.0F, 1.0F);
            GlStateManager.rotate(270.0F, 0.0F, 1.0F, 0.0F);
        }
        else if (bat.func_184613_cA())
        {
            super.rotateCorpse(bat, p_77043_2_, p_77043_3_, partialTicks);
            float f = (float)bat.func_184599_cB() + partialTicks;
            float f1 = MathHelper.clamp_float(f * f / 100.0F, 0.0F, 1.0F);
            GlStateManager.rotate(f1 * (-90.0F - bat.rotationPitch), 1.0F, 0.0F, 0.0F);
            Vec3d vec3d = bat.getLook(partialTicks);
            double d0 = bat.motionX * bat.motionX + bat.motionZ * bat.motionZ;
            double d1 = vec3d.xCoord * vec3d.xCoord + vec3d.zCoord * vec3d.zCoord;

            if (d0 > 0.0D && d1 > 0.0D)
            {
                double d2 = (bat.motionX * vec3d.xCoord + bat.motionZ * vec3d.zCoord) / (Math.sqrt(d0) * Math.sqrt(d1));
                double d3 = bat.motionX * vec3d.zCoord - bat.motionZ * vec3d.xCoord;
                GlStateManager.rotate((float)(Math.signum(d3) * Math.acos(d2)) * 180.0F / (float)Math.PI, 0.0F, 1.0F, 0.0F);
            }
        }
        else
        {
            super.rotateCorpse(bat, p_77043_2_, p_77043_3_, partialTicks);
        }
    }
}
