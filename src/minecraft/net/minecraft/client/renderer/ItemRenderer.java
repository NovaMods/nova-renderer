package net.minecraft.client.renderer;

import com.google.common.base.Objects;
import net.minecraft.block.Block;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.AbstractClientPlayer;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.renderer.block.model.ItemCameraTransforms;
import net.minecraft.client.renderer.entity.Render;
import net.minecraft.client.renderer.entity.RenderManager;
import net.minecraft.client.renderer.entity.RenderPlayer;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.storage.MapData;

public class ItemRenderer
{
    private static final ResourceLocation RES_MAP_BACKGROUND = new ResourceLocation("textures/map/map_background.png");
    private static final ResourceLocation RES_UNDERWATER_OVERLAY = new ResourceLocation("textures/misc/underwater.png");

    /** A reference to the Minecraft object. */
    private final Minecraft mc;
    private ItemStack field_187467_d;
    private ItemStack field_187468_e;
    private float field_187469_f;
    private float field_187470_g;
    private float field_187471_h;
    private float field_187472_i;
    private final RenderManager renderManager;
    private final RenderItem itemRenderer;

    public ItemRenderer(Minecraft mcIn)
    {
        this.mc = mcIn;
        this.renderManager = mcIn.getRenderManager();
        this.itemRenderer = mcIn.getRenderItem();
    }

    public void renderItem(EntityLivingBase entityIn, ItemStack heldStack, ItemCameraTransforms.TransformType transform)
    {
        this.func_187462_a(entityIn, heldStack, transform, false);
    }

    public void func_187462_a(EntityLivingBase p_187462_1_, ItemStack p_187462_2_, ItemCameraTransforms.TransformType p_187462_3_, boolean p_187462_4_)
    {
        if (p_187462_2_ != null)
        {
            Item item = p_187462_2_.getItem();
            Block block = Block.getBlockFromItem(item);
            GlStateManager.pushMatrix();
            boolean flag = this.itemRenderer.shouldRenderItemIn3D(p_187462_2_) && this.isBlockTranslucent(block);

            if (flag)
            {
                GlStateManager.depthMask(false);
            }

            this.itemRenderer.func_184392_a(p_187462_2_, p_187462_1_, p_187462_3_, p_187462_4_);

            if (flag)
            {
                GlStateManager.depthMask(true);
            }

            GlStateManager.popMatrix();
        }
    }

    /**
     * Returns true if given block is translucent
     */
    private boolean isBlockTranslucent(Block blockIn)
    {
        return blockIn != null && blockIn.getBlockLayer() == BlockRenderLayer.TRANSLUCENT;
    }

    /**
     * Rotate the render around X and Y
     */
    private void rotateArroundXAndY(float angle, float angleY)
    {
        GlStateManager.pushMatrix();
        GlStateManager.rotate(angle, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate(angleY, 0.0F, 1.0F, 0.0F);
        RenderHelper.enableStandardItemLighting();
        GlStateManager.popMatrix();
    }

    private void func_187464_b()
    {
        AbstractClientPlayer abstractclientplayer = this.mc.thePlayer;
        int i = this.mc.theWorld.getCombinedLight(new BlockPos(abstractclientplayer.posX, abstractclientplayer.posY + (double)abstractclientplayer.getEyeHeight(), abstractclientplayer.posZ), 0);
        float f = (float)(i & 65535);
        float f1 = (float)(i >> 16);
        OpenGlHelper.setLightmapTextureCoords(OpenGlHelper.lightmapTexUnit, f, f1);
    }

    private void func_187458_c(float p_187458_1_)
    {
        EntityPlayerSP entityplayersp = this.mc.thePlayer;
        float f = entityplayersp.prevRenderArmPitch + (entityplayersp.renderArmPitch - entityplayersp.prevRenderArmPitch) * p_187458_1_;
        float f1 = entityplayersp.prevRenderArmYaw + (entityplayersp.renderArmYaw - entityplayersp.prevRenderArmYaw) * p_187458_1_;
        GlStateManager.rotate((entityplayersp.rotationPitch - f) * 0.1F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate((entityplayersp.rotationYaw - f1) * 0.1F, 0.0F, 1.0F, 0.0F);
    }

    /**
     * Return the angle to render the Map
     */
    private float getMapAngleFromPitch(float pitch)
    {
        float f = 1.0F - pitch / 45.0F + 0.1F;
        f = MathHelper.clamp_float(f, 0.0F, 1.0F);
        f = -MathHelper.cos(f * (float)Math.PI) * 0.5F + 0.5F;
        return f;
    }

    private void func_187466_c()
    {
        if (!this.mc.thePlayer.isInvisible())
        {
            GlStateManager.disableCull();
            GlStateManager.pushMatrix();
            GlStateManager.rotate(90.0F, 0.0F, 1.0F, 0.0F);
            this.func_187455_a(EnumHandSide.RIGHT);
            this.func_187455_a(EnumHandSide.LEFT);
            GlStateManager.popMatrix();
            GlStateManager.enableCull();
        }
    }

    private void func_187455_a(EnumHandSide p_187455_1_)
    {
        this.mc.getTextureManager().bindTexture(this.mc.thePlayer.getLocationSkin());
        Render<AbstractClientPlayer> render = this.renderManager.<AbstractClientPlayer>getEntityRenderObject(this.mc.thePlayer);
        RenderPlayer renderplayer = (RenderPlayer)render;
        GlStateManager.pushMatrix();
        float f = p_187455_1_ == EnumHandSide.RIGHT ? 1.0F : -1.0F;
        GlStateManager.rotate(92.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.rotate(45.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate(f * -41.0F, 0.0F, 0.0F, 1.0F);
        GlStateManager.translate(f * 0.3F, -1.1F, 0.45F);

        if (p_187455_1_ == EnumHandSide.RIGHT)
        {
            renderplayer.renderRightArm(this.mc.thePlayer);
        }
        else
        {
            renderplayer.renderLeftArm(this.mc.thePlayer);
        }

        GlStateManager.popMatrix();
    }

    private void func_187465_a(float p_187465_1_, EnumHandSide p_187465_2_, float p_187465_3_, ItemStack p_187465_4_)
    {
        float f = p_187465_2_ == EnumHandSide.RIGHT ? 1.0F : -1.0F;
        GlStateManager.translate(f * 0.125F, -0.125F, 0.0F);

        if (!this.mc.thePlayer.isInvisible())
        {
            GlStateManager.pushMatrix();
            GlStateManager.rotate(f * 10.0F, 0.0F, 0.0F, 1.0F);
            this.func_187456_a(p_187465_1_, p_187465_3_, p_187465_2_);
            GlStateManager.popMatrix();
        }

        GlStateManager.pushMatrix();
        GlStateManager.translate(f * 0.51F, -0.08F + p_187465_1_ * -1.2F, -0.75F);
        float f1 = MathHelper.sqrt_float(p_187465_3_);
        float f2 = MathHelper.sin(f1 * (float)Math.PI);
        float f3 = -0.5F * f2;
        float f4 = 0.4F * MathHelper.sin(f1 * ((float)Math.PI * 2F));
        float f5 = -0.3F * MathHelper.sin(p_187465_3_ * (float)Math.PI);
        GlStateManager.translate(f * f3, f4 - 0.3F * f2, f5);
        GlStateManager.rotate(f2 * -45.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate(f * f2 * -30.0F, 0.0F, 1.0F, 0.0F);
        this.func_187461_a(p_187465_4_);
        GlStateManager.popMatrix();
    }

    private void func_187463_a(float p_187463_1_, float p_187463_2_, float p_187463_3_)
    {
        float f = MathHelper.sqrt_float(p_187463_3_);
        float f1 = -0.2F * MathHelper.sin(p_187463_3_ * (float)Math.PI);
        float f2 = -0.4F * MathHelper.sin(f * (float)Math.PI);
        GlStateManager.translate(0.0F, -f1 / 2.0F, f2);
        float f3 = this.getMapAngleFromPitch(p_187463_1_);
        GlStateManager.translate(0.0F, 0.04F + p_187463_2_ * -1.2F + f3 * -0.5F, -0.72F);
        GlStateManager.rotate(f3 * -85.0F, 1.0F, 0.0F, 0.0F);
        this.func_187466_c();
        float f4 = MathHelper.sin(f * (float)Math.PI);
        GlStateManager.rotate(f4 * 20.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.scale(2.0F, 2.0F, 2.0F);
        this.func_187461_a(this.field_187467_d);
    }

    private void func_187461_a(ItemStack p_187461_1_)
    {
        GlStateManager.rotate(180.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.rotate(180.0F, 0.0F, 0.0F, 1.0F);
        GlStateManager.scale(0.38F, 0.38F, 0.38F);
        GlStateManager.disableLighting();
        this.mc.getTextureManager().bindTexture(RES_MAP_BACKGROUND);
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        GlStateManager.translate(-0.5F, -0.5F, 0.0F);
        GlStateManager.scale(0.0078125F, 0.0078125F, 0.0078125F);
        vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX);
        vertexbuffer.pos(-7.0D, 135.0D, 0.0D).tex(0.0D, 1.0D).endVertex();
        vertexbuffer.pos(135.0D, 135.0D, 0.0D).tex(1.0D, 1.0D).endVertex();
        vertexbuffer.pos(135.0D, -7.0D, 0.0D).tex(1.0D, 0.0D).endVertex();
        vertexbuffer.pos(-7.0D, -7.0D, 0.0D).tex(0.0D, 0.0D).endVertex();
        tessellator.draw();
        MapData mapdata = Items.filled_map.getMapData(p_187461_1_, this.mc.theWorld);

        if (mapdata != null)
        {
            this.mc.entityRenderer.getMapItemRenderer().renderMap(mapdata, false);
        }

        GlStateManager.enableLighting();
    }

    private void func_187456_a(float p_187456_1_, float p_187456_2_, EnumHandSide p_187456_3_)
    {
        boolean flag = p_187456_3_ != EnumHandSide.LEFT;
        float f = flag ? 1.0F : -1.0F;
        float f1 = MathHelper.sqrt_float(p_187456_2_);
        float f2 = -0.3F * MathHelper.sin(f1 * (float)Math.PI);
        float f3 = 0.4F * MathHelper.sin(f1 * ((float)Math.PI * 2F));
        float f4 = -0.4F * MathHelper.sin(p_187456_2_ * (float)Math.PI);
        GlStateManager.translate(f * (f2 + 0.64000005F), f3 + -0.6F + p_187456_1_ * -0.6F, f4 + -0.71999997F);
        GlStateManager.rotate(f * 45.0F, 0.0F, 1.0F, 0.0F);
        float f5 = MathHelper.sin(p_187456_2_ * p_187456_2_ * (float)Math.PI);
        float f6 = MathHelper.sin(f1 * (float)Math.PI);
        GlStateManager.rotate(f * f6 * 70.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.rotate(f * f5 * -20.0F, 0.0F, 0.0F, 1.0F);
        AbstractClientPlayer abstractclientplayer = this.mc.thePlayer;
        this.mc.getTextureManager().bindTexture(abstractclientplayer.getLocationSkin());
        GlStateManager.translate(f * -1.0F, 3.6F, 3.5F);
        GlStateManager.rotate(f * 120.0F, 0.0F, 0.0F, 1.0F);
        GlStateManager.rotate(200.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate(f * -135.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.translate(f * 5.6F, 0.0F, 0.0F);
        RenderPlayer renderplayer = (RenderPlayer)this.renderManager.getEntityRenderObject(abstractclientplayer);
        GlStateManager.disableCull();

        if (flag)
        {
            renderplayer.renderRightArm(abstractclientplayer);
        }
        else
        {
            renderplayer.renderLeftArm(abstractclientplayer);
        }

        GlStateManager.enableCull();
    }

    private void func_187454_a(float p_187454_1_, EnumHandSide p_187454_2_, ItemStack p_187454_3_)
    {
        float f = (float)this.mc.thePlayer.getItemInUseCount() - p_187454_1_ + 1.0F;
        float f1 = f / (float)p_187454_3_.getMaxItemUseDuration();

        if (f1 < 0.8F)
        {
            float f2 = MathHelper.abs(MathHelper.cos(f / 4.0F * (float)Math.PI) * 0.1F);
            GlStateManager.translate(0.0F, f2, 0.0F);
        }

        float f3 = 1.0F - (float)Math.pow((double)f1, 27.0D);
        int i = p_187454_2_ == EnumHandSide.RIGHT ? 1 : -1;
        GlStateManager.translate(f3 * 0.6F * (float)i, f3 * -0.5F, f3 * 0.0F);
        GlStateManager.rotate((float)i * f3 * 90.0F, 0.0F, 1.0F, 0.0F);
        GlStateManager.rotate(f3 * 10.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate((float)i * f3 * 30.0F, 0.0F, 0.0F, 1.0F);
    }

    private void func_187453_a(EnumHandSide p_187453_1_, float p_187453_2_)
    {
        int i = p_187453_1_ == EnumHandSide.RIGHT ? 1 : -1;
        float f = MathHelper.sin(p_187453_2_ * p_187453_2_ * (float)Math.PI);
        GlStateManager.rotate((float)i * (45.0F + f * -20.0F), 0.0F, 1.0F, 0.0F);
        float f1 = MathHelper.sin(MathHelper.sqrt_float(p_187453_2_) * (float)Math.PI);
        GlStateManager.rotate((float)i * f1 * -20.0F, 0.0F, 0.0F, 1.0F);
        GlStateManager.rotate(f1 * -80.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager.rotate((float)i * -45.0F, 0.0F, 1.0F, 0.0F);
    }

    private void func_187459_b(EnumHandSide p_187459_1_, float p_187459_2_)
    {
        int i = p_187459_1_ == EnumHandSide.RIGHT ? 1 : -1;
        GlStateManager.translate((float)i * 0.56F, -0.52F + p_187459_2_ * -0.6F, -0.72F);
    }

    /**
     * Renders the active item in the player's hand when in first person mode. Args: partialTickTime
     */
    public void renderItemInFirstPerson(float partialTicks)
    {
        AbstractClientPlayer abstractclientplayer = this.mc.thePlayer;
        float f = abstractclientplayer.getSwingProgress(partialTicks);
        EnumHand enumhand = (EnumHand)Objects.firstNonNull(abstractclientplayer.field_184622_au, EnumHand.MAIN_HAND);
        float f1 = abstractclientplayer.prevRotationPitch + (abstractclientplayer.rotationPitch - abstractclientplayer.prevRotationPitch) * partialTicks;
        float f2 = abstractclientplayer.prevRotationYaw + (abstractclientplayer.rotationYaw - abstractclientplayer.prevRotationYaw) * partialTicks;
        boolean flag = true;
        boolean flag1 = true;

        if (abstractclientplayer.func_184587_cr())
        {
            ItemStack itemstack = abstractclientplayer.func_184607_cu();

            if (itemstack.getItem() == Items.bow)
            {
                EnumHand enumhand1 = abstractclientplayer.func_184600_cs();
                flag = enumhand1 == EnumHand.MAIN_HAND;
                flag1 = !flag;
            }
        }

        this.rotateArroundXAndY(f1, f2);
        this.func_187464_b();
        this.func_187458_c(partialTicks);
        GlStateManager.enableRescaleNormal();

        if (flag)
        {
            float f3 = enumhand == EnumHand.MAIN_HAND ? f : 0.0F;
            float f5 = 1.0F - (this.field_187470_g + (this.field_187469_f - this.field_187470_g) * partialTicks);
            this.func_187457_a(abstractclientplayer, partialTicks, f1, EnumHand.MAIN_HAND, f3, this.field_187467_d, f5);
        }

        if (flag1)
        {
            float f4 = enumhand == EnumHand.OFF_HAND ? f : 0.0F;
            float f6 = 1.0F - (this.field_187472_i + (this.field_187471_h - this.field_187472_i) * partialTicks);
            this.func_187457_a(abstractclientplayer, partialTicks, f1, EnumHand.OFF_HAND, f4, this.field_187468_e, f6);
        }

        GlStateManager.disableRescaleNormal();
        RenderHelper.disableStandardItemLighting();
    }

    public void func_187457_a(AbstractClientPlayer p_187457_1_, float p_187457_2_, float p_187457_3_, EnumHand p_187457_4_, float p_187457_5_, ItemStack p_187457_6_, float p_187457_7_)
    {
        boolean flag = p_187457_4_ == EnumHand.MAIN_HAND;
        EnumHandSide enumhandside = flag ? p_187457_1_.getPrimaryHand() : p_187457_1_.getPrimaryHand().opposite();
        GlStateManager.pushMatrix();

        if (p_187457_6_ == null)
        {
            if (flag && !p_187457_1_.isInvisible())
            {
                this.func_187456_a(p_187457_7_, p_187457_5_, enumhandside);
            }
        }
        else if (p_187457_6_.getItem() == Items.filled_map)
        {
            if (flag && this.field_187468_e == null)
            {
                this.func_187463_a(p_187457_3_, p_187457_7_, p_187457_5_);
            }
            else
            {
                this.func_187465_a(p_187457_7_, enumhandside, p_187457_5_, p_187457_6_);
            }
        }
        else
        {
            boolean flag1 = enumhandside == EnumHandSide.RIGHT;

            if (p_187457_1_.func_184587_cr() && p_187457_1_.getItemInUseCount() > 0 && p_187457_1_.func_184600_cs() == p_187457_4_)
            {
                int j = flag1 ? 1 : -1;

                switch (p_187457_6_.getItemUseAction())
                {
                    case NONE:
                        this.func_187459_b(enumhandside, p_187457_7_);
                        break;

                    case EAT:
                    case DRINK:
                        this.func_187454_a(p_187457_2_, enumhandside, p_187457_6_);
                        this.func_187459_b(enumhandside, p_187457_7_);
                        break;

                    case BLOCK:
                        this.func_187459_b(enumhandside, p_187457_7_);
                        break;

                    case BOW:
                        this.func_187459_b(enumhandside, p_187457_7_);
                        GlStateManager.translate((float)j * -0.2785682F, 0.18344387F, 0.15731531F);
                        GlStateManager.rotate(-13.935F, 1.0F, 0.0F, 0.0F);
                        GlStateManager.rotate((float)j * 35.3F, 0.0F, 1.0F, 0.0F);
                        GlStateManager.rotate((float)j * -9.785F, 0.0F, 0.0F, 1.0F);
                        float f5 = (float)p_187457_6_.getMaxItemUseDuration() - ((float)this.mc.thePlayer.getItemInUseCount() - p_187457_2_ + 1.0F);
                        float f6 = f5 / 20.0F;
                        f6 = (f6 * f6 + f6 * 2.0F) / 3.0F;

                        if (f6 > 1.0F)
                        {
                            f6 = 1.0F;
                        }

                        if (f6 > 0.1F)
                        {
                            float f7 = MathHelper.sin((f5 - 0.1F) * 1.3F);
                            float f3 = f6 - 0.1F;
                            float f4 = f7 * f3;
                            GlStateManager.translate(f4 * 0.0F, f4 * 0.004F, f4 * 0.0F);
                        }

                        GlStateManager.translate(f6 * 0.0F, f6 * 0.0F, f6 * 0.04F);
                        GlStateManager.scale(1.0F, 1.0F, 1.0F + f6 * 0.2F);
                        GlStateManager.rotate((float)j * 45.0F, 0.0F, -1.0F, 0.0F);
                }
            }
            else
            {
                float f = -0.4F * MathHelper.sin(MathHelper.sqrt_float(p_187457_5_) * (float)Math.PI);
                float f1 = 0.2F * MathHelper.sin(MathHelper.sqrt_float(p_187457_5_) * ((float)Math.PI * 2F));
                float f2 = -0.2F * MathHelper.sin(p_187457_5_ * (float)Math.PI);
                int i = flag1 ? 1 : -1;
                GlStateManager.translate((float)i * f, f1, f2);
                this.func_187459_b(enumhandside, p_187457_7_);
                this.func_187453_a(enumhandside, p_187457_5_);
            }

            this.func_187462_a(p_187457_1_, p_187457_6_, flag1 ? ItemCameraTransforms.TransformType.FIRST_PERSON_RIGHT_HAND : ItemCameraTransforms.TransformType.FIRST_PERSON_LEFT_HAND, !flag1);
        }

        GlStateManager.popMatrix();
    }

    /**
     * Renders all the overlays that are in first person mode. Args: partialTickTime
     */
    public void renderOverlays(float partialTicks)
    {
        GlStateManager.disableAlpha();

        if (this.mc.thePlayer.isEntityInsideOpaqueBlock())
        {
            IBlockState iblockstate = this.mc.theWorld.getBlockState(new BlockPos(this.mc.thePlayer));
            EntityPlayer entityplayer = this.mc.thePlayer;

            for (int i = 0; i < 8; ++i)
            {
                double d0 = entityplayer.posX + (double)(((float)((i >> 0) % 2) - 0.5F) * entityplayer.width * 0.8F);
                double d1 = entityplayer.posY + (double)(((float)((i >> 1) % 2) - 0.5F) * 0.1F);
                double d2 = entityplayer.posZ + (double)(((float)((i >> 2) % 2) - 0.5F) * entityplayer.width * 0.8F);
                BlockPos blockpos = new BlockPos(d0, d1 + (double)entityplayer.getEyeHeight(), d2);
                IBlockState iblockstate1 = this.mc.theWorld.getBlockState(blockpos);

                if (iblockstate1.getBlock().isVisuallyOpaque())
                {
                    iblockstate = iblockstate1;
                }
            }

            if (iblockstate.getRenderType() != EnumBlockRenderType.INVISIBLE)
            {
                this.renderBlockInHand(partialTicks, this.mc.getBlockRendererDispatcher().getBlockModelShapes().getTexture(iblockstate));
            }
        }

        if (!this.mc.thePlayer.isSpectator())
        {
            if (this.mc.thePlayer.isInsideOfMaterial(Material.water))
            {
                this.renderWaterOverlayTexture(partialTicks);
            }

            if (this.mc.thePlayer.isBurning())
            {
                this.renderFireInFirstPerson(partialTicks);
            }
        }

        GlStateManager.enableAlpha();
    }

    /**
     * Render the block in the player's hand
     */
    private void renderBlockInHand(float partialTicks, TextureAtlasSprite atlas)
    {
        this.mc.getTextureManager().bindTexture(TextureMap.locationBlocksTexture);
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        float f = 0.1F;
        GlStateManager.color(0.1F, 0.1F, 0.1F, 0.5F);
        GlStateManager.pushMatrix();
        float f1 = -1.0F;
        float f2 = 1.0F;
        float f3 = -1.0F;
        float f4 = 1.0F;
        float f5 = -0.5F;
        float f6 = atlas.getMinU();
        float f7 = atlas.getMaxU();
        float f8 = atlas.getMinV();
        float f9 = atlas.getMaxV();
        vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX);
        vertexbuffer.pos(-1.0D, -1.0D, -0.5D).tex((double)f7, (double)f9).endVertex();
        vertexbuffer.pos(1.0D, -1.0D, -0.5D).tex((double)f6, (double)f9).endVertex();
        vertexbuffer.pos(1.0D, 1.0D, -0.5D).tex((double)f6, (double)f8).endVertex();
        vertexbuffer.pos(-1.0D, 1.0D, -0.5D).tex((double)f7, (double)f8).endVertex();
        tessellator.draw();
        GlStateManager.popMatrix();
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
    }

    /**
     * Renders a texture that warps around based on the direction the player is looking. Texture needs to be bound
     * before being called. Used for the water overlay. Args: parialTickTime
     */
    private void renderWaterOverlayTexture(float partialTicks)
    {
        this.mc.getTextureManager().bindTexture(RES_UNDERWATER_OVERLAY);
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        float f = this.mc.thePlayer.getBrightness(partialTicks);
        GlStateManager.color(f, f, f, 0.5F);
        GlStateManager.enableBlend();
        GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA, GlStateManager.SourceFactor.ONE, GlStateManager.DestFactor.ZERO);
        GlStateManager.pushMatrix();
        float f1 = 4.0F;
        float f2 = -1.0F;
        float f3 = 1.0F;
        float f4 = -1.0F;
        float f5 = 1.0F;
        float f6 = -0.5F;
        float f7 = -this.mc.thePlayer.rotationYaw / 64.0F;
        float f8 = this.mc.thePlayer.rotationPitch / 64.0F;
        vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX);
        vertexbuffer.pos(-1.0D, -1.0D, -0.5D).tex((double)(4.0F + f7), (double)(4.0F + f8)).endVertex();
        vertexbuffer.pos(1.0D, -1.0D, -0.5D).tex((double)(0.0F + f7), (double)(4.0F + f8)).endVertex();
        vertexbuffer.pos(1.0D, 1.0D, -0.5D).tex((double)(0.0F + f7), (double)(0.0F + f8)).endVertex();
        vertexbuffer.pos(-1.0D, 1.0D, -0.5D).tex((double)(4.0F + f7), (double)(0.0F + f8)).endVertex();
        tessellator.draw();
        GlStateManager.popMatrix();
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        GlStateManager.disableBlend();
    }

    /**
     * Renders the fire on the screen for first person mode. Arg: partialTickTime
     */
    private void renderFireInFirstPerson(float partialTicks)
    {
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        GlStateManager.color(1.0F, 1.0F, 1.0F, 0.9F);
        GlStateManager.depthFunc(519);
        GlStateManager.depthMask(false);
        GlStateManager.enableBlend();
        GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA, GlStateManager.SourceFactor.ONE, GlStateManager.DestFactor.ZERO);
        float f = 1.0F;

        for (int i = 0; i < 2; ++i)
        {
            GlStateManager.pushMatrix();
            TextureAtlasSprite textureatlassprite = this.mc.getTextureMapBlocks().getAtlasSprite("minecraft:blocks/fire_layer_1");
            this.mc.getTextureManager().bindTexture(TextureMap.locationBlocksTexture);
            float f1 = textureatlassprite.getMinU();
            float f2 = textureatlassprite.getMaxU();
            float f3 = textureatlassprite.getMinV();
            float f4 = textureatlassprite.getMaxV();
            float f5 = -0.5F;
            float f6 = 0.5F;
            float f7 = -0.5F;
            float f8 = 0.5F;
            float f9 = -0.5F;
            GlStateManager.translate((float)(-(i * 2 - 1)) * 0.24F, -0.3F, 0.0F);
            GlStateManager.rotate((float)(i * 2 - 1) * 10.0F, 0.0F, 1.0F, 0.0F);
            vertexbuffer.begin(7, DefaultVertexFormats.POSITION_TEX);
            vertexbuffer.pos(-0.5D, -0.5D, -0.5D).tex((double)f2, (double)f4).endVertex();
            vertexbuffer.pos(0.5D, -0.5D, -0.5D).tex((double)f1, (double)f4).endVertex();
            vertexbuffer.pos(0.5D, 0.5D, -0.5D).tex((double)f1, (double)f3).endVertex();
            vertexbuffer.pos(-0.5D, 0.5D, -0.5D).tex((double)f2, (double)f3).endVertex();
            tessellator.draw();
            GlStateManager.popMatrix();
        }

        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        GlStateManager.disableBlend();
        GlStateManager.depthMask(true);
        GlStateManager.depthFunc(515);
    }

    public void updateEquippedItem()
    {
        this.field_187470_g = this.field_187469_f;
        this.field_187472_i = this.field_187471_h;
        EntityPlayerSP entityplayersp = this.mc.thePlayer;
        ItemStack itemstack = entityplayersp.getHeldItemMainhand();
        ItemStack itemstack1 = entityplayersp.getHeldItemOffhand();

        if (entityplayersp.func_184838_M())
        {
            this.field_187469_f = MathHelper.clamp_float(this.field_187469_f - 0.4F, 0.0F, 1.0F);
            this.field_187471_h = MathHelper.clamp_float(this.field_187471_h - 0.4F, 0.0F, 1.0F);
        }
        else
        {
            float f = entityplayersp.func_184825_o(1.0F);
            this.field_187469_f += MathHelper.clamp_float((Objects.equal(this.field_187467_d, itemstack) ? f * f * f : 0.0F) - this.field_187469_f, -0.4F, 0.4F);
            this.field_187471_h += MathHelper.clamp_float((float)(Objects.equal(this.field_187468_e, itemstack1) ? 1 : 0) - this.field_187471_h, -0.4F, 0.4F);
        }

        if (this.field_187469_f < 0.1F)
        {
            this.field_187467_d = itemstack;
        }

        if (this.field_187471_h < 0.1F)
        {
            this.field_187468_e = itemstack1;
        }
    }

    public void func_187460_a(EnumHand p_187460_1_)
    {
        if (p_187460_1_ == EnumHand.MAIN_HAND)
        {
            this.field_187469_f = 0.0F;
        }
        else
        {
            this.field_187471_h = 0.0F;
        }
    }
}
