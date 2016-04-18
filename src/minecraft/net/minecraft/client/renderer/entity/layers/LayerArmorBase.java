package net.minecraft.client.renderer.entity.layers;

import com.google.common.collect.Maps;
import java.util.Map;
import net.minecraft.client.model.ModelBase;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.client.renderer.entity.RenderLivingBase;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemStack;
import net.minecraft.util.ResourceLocation;

public abstract class LayerArmorBase<T extends ModelBase> implements LayerRenderer<EntityLivingBase>
{
    protected static final ResourceLocation ENCHANTED_ITEM_GLINT_RES = new ResourceLocation("textures/misc/enchanted_item_glint.png");
    protected T modelLeggings;
    protected T modelArmor;
    private final RenderLivingBase<?> renderer;
    private float alpha = 1.0F;
    private float colorR = 1.0F;
    private float colorG = 1.0F;
    private float colorB = 1.0F;
    private boolean skipRenderGlint;
    private static final Map<String, ResourceLocation> ARMOR_TEXTURE_RES_MAP = Maps.<String, ResourceLocation>newHashMap();

    public LayerArmorBase(RenderLivingBase<?> rendererIn)
    {
        this.renderer = rendererIn;
        this.initArmor();
    }

    public void doRenderLayer(EntityLivingBase entitylivingbaseIn, float p_177141_2_, float p_177141_3_, float partialTicks, float p_177141_5_, float p_177141_6_, float p_177141_7_, float scale)
    {
        this.func_188361_a(entitylivingbaseIn, p_177141_2_, p_177141_3_, partialTicks, p_177141_5_, p_177141_6_, p_177141_7_, scale, EntityEquipmentSlot.CHEST);
        this.func_188361_a(entitylivingbaseIn, p_177141_2_, p_177141_3_, partialTicks, p_177141_5_, p_177141_6_, p_177141_7_, scale, EntityEquipmentSlot.LEGS);
        this.func_188361_a(entitylivingbaseIn, p_177141_2_, p_177141_3_, partialTicks, p_177141_5_, p_177141_6_, p_177141_7_, scale, EntityEquipmentSlot.FEET);
        this.func_188361_a(entitylivingbaseIn, p_177141_2_, p_177141_3_, partialTicks, p_177141_5_, p_177141_6_, p_177141_7_, scale, EntityEquipmentSlot.HEAD);
    }

    public boolean shouldCombineTextures()
    {
        return false;
    }

    private void func_188361_a(EntityLivingBase p_188361_1_, float p_188361_2_, float p_188361_3_, float p_188361_4_, float p_188361_5_, float p_188361_6_, float p_188361_7_, float p_188361_8_, EntityEquipmentSlot p_188361_9_)
    {
        ItemStack itemstack = this.func_188362_a(p_188361_1_, p_188361_9_);

        if (itemstack != null && itemstack.getItem() instanceof ItemArmor)
        {
            ItemArmor itemarmor = (ItemArmor)itemstack.getItem();

            if (itemarmor.func_185083_B_() == p_188361_9_)
            {
                T t = this.func_188360_a(p_188361_9_);
                t.setModelAttributes(this.renderer.getMainModel());
                t.setLivingAnimations(p_188361_1_, p_188361_2_, p_188361_3_, p_188361_4_);
                this.func_188359_a(t, p_188361_9_);
                boolean flag = this.func_188363_b(p_188361_9_);
                this.renderer.bindTexture(this.getArmorResource(itemarmor, flag));

                switch (itemarmor.getArmorMaterial())
                {
                    case LEATHER:
                        int i = itemarmor.getColor(itemstack);
                        float f = (float)(i >> 16 & 255) / 255.0F;
                        float f1 = (float)(i >> 8 & 255) / 255.0F;
                        float f2 = (float)(i & 255) / 255.0F;
                        GlStateManager.color(this.colorR * f, this.colorG * f1, this.colorB * f2, this.alpha);
                        t.render(p_188361_1_, p_188361_2_, p_188361_3_, p_188361_5_, p_188361_6_, p_188361_7_, p_188361_8_);
                        this.renderer.bindTexture(this.getArmorResource(itemarmor, flag, "overlay"));

                    case CHAIN:
                    case IRON:
                    case GOLD:
                    case DIAMOND:
                        GlStateManager.color(this.colorR, this.colorG, this.colorB, this.alpha);
                        t.render(p_188361_1_, p_188361_2_, p_188361_3_, p_188361_5_, p_188361_6_, p_188361_7_, p_188361_8_);

                    default:
                        if (!this.skipRenderGlint && itemstack.isItemEnchanted())
                        {
                            func_188364_a(this.renderer, p_188361_1_, t, p_188361_2_, p_188361_3_, p_188361_4_, p_188361_5_, p_188361_6_, p_188361_7_, p_188361_8_);
                        }
                }
            }
        }
    }

    public ItemStack func_188362_a(EntityLivingBase p_188362_1_, EntityEquipmentSlot p_188362_2_)
    {
        return p_188362_1_.getItemStackFromSlot(p_188362_2_);
    }

    public T func_188360_a(EntityEquipmentSlot p_188360_1_)
    {
        return (T)(this.func_188363_b(p_188360_1_) ? this.modelLeggings : this.modelArmor);
    }

    private boolean func_188363_b(EntityEquipmentSlot p_188363_1_)
    {
        return p_188363_1_ == EntityEquipmentSlot.LEGS;
    }

    public static void func_188364_a(RenderLivingBase<?> p_188364_0_, EntityLivingBase p_188364_1_, ModelBase p_188364_2_, float p_188364_3_, float p_188364_4_, float p_188364_5_, float p_188364_6_, float p_188364_7_, float p_188364_8_, float p_188364_9_)
    {
        float f = (float)p_188364_1_.ticksExisted + p_188364_5_;
        p_188364_0_.bindTexture(ENCHANTED_ITEM_GLINT_RES);
        GlStateManager.enableBlend();
        GlStateManager.depthFunc(514);
        GlStateManager.depthMask(false);
        float f1 = 0.5F;
        GlStateManager.color(f1, f1, f1, 1.0F);

        for (int i = 0; i < 2; ++i)
        {
            GlStateManager.disableLighting();
            GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_COLOR, GlStateManager.DestFactor.ONE);
            float f2 = 0.76F;
            GlStateManager.color(0.5F * f2, 0.25F * f2, 0.8F * f2, 1.0F);
            GlStateManager.matrixMode(5890);
            GlStateManager.loadIdentity();
            float f3 = 0.33333334F;
            GlStateManager.scale(f3, f3, f3);
            GlStateManager.rotate(30.0F - (float)i * 60.0F, 0.0F, 0.0F, 1.0F);
            GlStateManager.translate(0.0F, f * (0.001F + (float)i * 0.003F) * 20.0F, 0.0F);
            GlStateManager.matrixMode(5888);
            p_188364_2_.render(p_188364_1_, p_188364_3_, p_188364_4_, p_188364_6_, p_188364_7_, p_188364_8_, p_188364_9_);
        }

        GlStateManager.matrixMode(5890);
        GlStateManager.loadIdentity();
        GlStateManager.matrixMode(5888);
        GlStateManager.enableLighting();
        GlStateManager.depthMask(true);
        GlStateManager.depthFunc(515);
        GlStateManager.disableBlend();
    }

    private ResourceLocation getArmorResource(ItemArmor p_177181_1_, boolean p_177181_2_)
    {
        return this.getArmorResource(p_177181_1_, p_177181_2_, (String)null);
    }

    private ResourceLocation getArmorResource(ItemArmor p_177178_1_, boolean p_177178_2_, String p_177178_3_)
    {
        String s = String.format("textures/models/armor/%s_layer_%d%s.png", new Object[] {p_177178_1_.getArmorMaterial().getName(), Integer.valueOf(p_177178_2_ ? 2 : 1), p_177178_3_ == null ? "" : String.format("_%s", new Object[]{p_177178_3_})});
        ResourceLocation resourcelocation = (ResourceLocation)ARMOR_TEXTURE_RES_MAP.get(s);

        if (resourcelocation == null)
        {
            resourcelocation = new ResourceLocation(s);
            ARMOR_TEXTURE_RES_MAP.put(s, resourcelocation);
        }

        return resourcelocation;
    }

    protected abstract void initArmor();

    protected abstract void func_188359_a(T p_188359_1_, EntityEquipmentSlot p_188359_2_);
}
