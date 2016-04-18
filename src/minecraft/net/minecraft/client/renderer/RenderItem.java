package net.minecraft.client.renderer;

import java.util.List;
import java.util.concurrent.Callable;
import net.minecraft.block.Block;
import net.minecraft.block.BlockDirt;
import net.minecraft.block.BlockDoublePlant;
import net.minecraft.block.BlockFlower;
import net.minecraft.block.BlockHugeMushroom;
import net.minecraft.block.BlockPlanks;
import net.minecraft.block.BlockPrismarine;
import net.minecraft.block.BlockQuartz;
import net.minecraft.block.BlockRedSandstone;
import net.minecraft.block.BlockSand;
import net.minecraft.block.BlockSandStone;
import net.minecraft.block.BlockSilverfish;
import net.minecraft.block.BlockStone;
import net.minecraft.block.BlockStoneBrick;
import net.minecraft.block.BlockStoneSlab;
import net.minecraft.block.BlockStoneSlabNew;
import net.minecraft.block.BlockTallGrass;
import net.minecraft.block.BlockWall;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.entity.EntityPlayerSP;
import net.minecraft.client.gui.FontRenderer;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.client.renderer.block.model.ItemCameraTransforms;
import net.minecraft.client.renderer.block.model.ItemTransformVec3f;
import net.minecraft.client.renderer.block.model.ModelManager;
import net.minecraft.client.renderer.block.model.ModelResourceLocation;
import net.minecraft.client.renderer.color.ItemColors;
import net.minecraft.client.renderer.texture.TextureManager;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.renderer.texture.TextureUtil;
import net.minecraft.client.renderer.tileentity.TileEntityItemStackRenderer;
import net.minecraft.client.renderer.vertex.DefaultVertexFormats;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.client.resources.IResourceManagerReloadListener;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.EnumDyeColor;
import net.minecraft.item.Item;
import net.minecraft.item.ItemFishFood;
import net.minecraft.item.ItemStack;
import net.minecraft.tileentity.TileEntityStructure;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ReportedException;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3i;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.world.World;

public class RenderItem implements IResourceManagerReloadListener
{
    private static final ResourceLocation RES_ITEM_GLINT = new ResourceLocation("textures/misc/enchanted_item_glint.png");

    /** False when the renderer is rendering the item's effects into a GUI */
    private boolean notRenderingEffectsInGUI = true;

    /** Defines the zLevel of rendering of item on GUI. */
    public float zLevel;
    private final ItemModelMesher itemModelMesher;
    private final TextureManager textureManager;
    private final ItemColors field_184395_f;

    public RenderItem(TextureManager p_i46552_1_, ModelManager p_i46552_2_, ItemColors p_i46552_3_)
    {
        this.textureManager = p_i46552_1_;
        this.itemModelMesher = new ItemModelMesher(p_i46552_2_);
        this.registerItems();
        this.field_184395_f = p_i46552_3_;
    }

    /**
     * False when the renderer is rendering the item's effects into a GUI
     */
    public void isNotRenderingEffectsInGUI(boolean isNot)
    {
        this.notRenderingEffectsInGUI = isNot;
    }

    public ItemModelMesher getItemModelMesher()
    {
        return this.itemModelMesher;
    }

    protected void registerItem(Item itm, int subType, String identifier)
    {
        this.itemModelMesher.register(itm, subType, new ModelResourceLocation(identifier, "inventory"));
    }

    protected void registerBlock(Block blk, int subType, String identifier)
    {
        this.registerItem(Item.getItemFromBlock(blk), subType, identifier);
    }

    private void registerBlock(Block blk, String identifier)
    {
        this.registerBlock(blk, 0, identifier);
    }

    private void registerItem(Item itm, String identifier)
    {
        this.registerItem(itm, 0, identifier);
    }

    private void renderModel(IBakedModel model, ItemStack stack)
    {
        this.renderModel(model, -1, stack);
    }

    private void renderModel(IBakedModel model, int color)
    {
        this.renderModel(model, color, (ItemStack)null);
    }

    private void renderModel(IBakedModel model, int color, ItemStack stack)
    {
        Tessellator tessellator = Tessellator.getInstance();
        VertexBuffer vertexbuffer = tessellator.getBuffer();
        vertexbuffer.begin(7, DefaultVertexFormats.ITEM);

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            this.renderQuads(vertexbuffer, model.getQuads((IBlockState)null, enumfacing, 0L), color, stack);
        }

        this.renderQuads(vertexbuffer, model.getQuads((IBlockState)null, (EnumFacing)null, 0L), color, stack);
        tessellator.draw();
    }

    public void renderItem(ItemStack stack, IBakedModel model)
    {
        if (stack != null)
        {
            GlStateManager.pushMatrix();
            GlStateManager.translate(-0.5F, -0.5F, -0.5F);

            if (model.isBuiltInRenderer())
            {
                GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
                GlStateManager.enableRescaleNormal();
                TileEntityItemStackRenderer.instance.renderByItem(stack);
            }
            else
            {
                this.renderModel(model, stack);

                if (stack.hasEffect())
                {
                    this.renderEffect(model);
                }
            }

            GlStateManager.popMatrix();
        }
    }

    private void renderEffect(IBakedModel model)
    {
        GlStateManager.depthMask(false);
        GlStateManager.depthFunc(514);
        GlStateManager.disableLighting();
        GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_COLOR, GlStateManager.DestFactor.ONE);
        this.textureManager.bindTexture(RES_ITEM_GLINT);
        GlStateManager.matrixMode(5890);
        GlStateManager.pushMatrix();
        GlStateManager.scale(8.0F, 8.0F, 8.0F);
        float f = (float)(Minecraft.getSystemTime() % 3000L) / 3000.0F / 8.0F;
        GlStateManager.translate(f, 0.0F, 0.0F);
        GlStateManager.rotate(-50.0F, 0.0F, 0.0F, 1.0F);
        this.renderModel(model, -8372020);
        GlStateManager.popMatrix();
        GlStateManager.pushMatrix();
        GlStateManager.scale(8.0F, 8.0F, 8.0F);
        float f1 = (float)(Minecraft.getSystemTime() % 4873L) / 4873.0F / 8.0F;
        GlStateManager.translate(-f1, 0.0F, 0.0F);
        GlStateManager.rotate(10.0F, 0.0F, 0.0F, 1.0F);
        this.renderModel(model, -8372020);
        GlStateManager.popMatrix();
        GlStateManager.matrixMode(5888);
        GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA);
        GlStateManager.enableLighting();
        GlStateManager.depthFunc(515);
        GlStateManager.depthMask(true);
        this.textureManager.bindTexture(TextureMap.locationBlocksTexture);
    }

    private void putQuadNormal(VertexBuffer renderer, BakedQuad quad)
    {
        Vec3i vec3i = quad.getFace().getDirectionVec();
        renderer.putNormal((float)vec3i.getX(), (float)vec3i.getY(), (float)vec3i.getZ());
    }

    private void renderQuad(VertexBuffer renderer, BakedQuad quad, int color)
    {
        renderer.addVertexData(quad.getVertexData());
        renderer.putColor4(color);
        this.putQuadNormal(renderer, quad);
    }

    private void renderQuads(VertexBuffer renderer, List<BakedQuad> quads, int color, ItemStack stack)
    {
        boolean flag = color == -1 && stack != null;
        int i = 0;

        for (int j = quads.size(); i < j; ++i)
        {
            BakedQuad bakedquad = (BakedQuad)quads.get(i);
            int k = color;

            if (flag && bakedquad.hasTintIndex())
            {
                k = this.field_184395_f.func_186728_a(stack, bakedquad.getTintIndex());

                if (EntityRenderer.anaglyphEnable)
                {
                    k = TextureUtil.anaglyphColor(k);
                }

                k = k | -16777216;
            }

            this.renderQuad(renderer, bakedquad, k);
        }
    }

    public boolean shouldRenderItemIn3D(ItemStack stack)
    {
        IBakedModel ibakedmodel = this.itemModelMesher.getItemModel(stack);
        return ibakedmodel == null ? false : ibakedmodel.isGui3d();
    }

    public void renderItem(ItemStack stack, ItemCameraTransforms.TransformType cameraTransformType)
    {
        if (stack != null)
        {
            IBakedModel ibakedmodel = this.getItemModelWithOverrides(stack, (World)null, (EntityLivingBase)null);
            this.func_184394_a(stack, ibakedmodel, cameraTransformType, false);
        }
    }

    public IBakedModel getItemModelWithOverrides(ItemStack p_184393_1_, World p_184393_2_, EntityLivingBase p_184393_3_)
    {
        IBakedModel ibakedmodel = this.itemModelMesher.getItemModel(p_184393_1_);
        Item item = p_184393_1_.getItem();

        if (item != null && item.hasCustomProperties())
        {
            ResourceLocation resourcelocation = ibakedmodel.getOverrides().applyOverride(p_184393_1_, p_184393_2_, p_184393_3_);
            return resourcelocation == null ? ibakedmodel : this.itemModelMesher.getModelManager().getModel(new ModelResourceLocation(resourcelocation, "inventory"));
        }
        else
        {
            return ibakedmodel;
        }
    }

    public void func_184392_a(ItemStack p_184392_1_, EntityLivingBase p_184392_2_, ItemCameraTransforms.TransformType p_184392_3_, boolean p_184392_4_)
    {
        if (p_184392_1_ != null && p_184392_2_ != null && p_184392_1_.getItem() != null)
        {
            IBakedModel ibakedmodel = this.getItemModelWithOverrides(p_184392_1_, p_184392_2_.worldObj, p_184392_2_);
            this.func_184394_a(p_184392_1_, ibakedmodel, p_184392_3_, p_184392_4_);
        }
    }

    protected void func_184394_a(ItemStack p_184394_1_, IBakedModel p_184394_2_, ItemCameraTransforms.TransformType p_184394_3_, boolean p_184394_4_)
    {
        if (p_184394_1_.getItem() != null)
        {
            this.textureManager.bindTexture(TextureMap.locationBlocksTexture);
            this.textureManager.getTexture(TextureMap.locationBlocksTexture).setBlurMipmap(false, false);
            GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
            GlStateManager.enableRescaleNormal();
            GlStateManager.alphaFunc(516, 0.1F);
            GlStateManager.enableBlend();
            GlStateManager.tryBlendFuncSeparate(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA, GlStateManager.SourceFactor.ONE, GlStateManager.DestFactor.ZERO);
            GlStateManager.pushMatrix();
            ItemCameraTransforms itemcameratransforms = p_184394_2_.getItemCameraTransforms();
            ItemCameraTransforms.func_188034_a(itemcameratransforms.getTransform(p_184394_3_), p_184394_4_);

            if (this.isThereOneNegativeScale(itemcameratransforms.getTransform(p_184394_3_)))
            {
                GlStateManager.cullFace(GlStateManager.CullFace.FRONT);
            }

            this.renderItem(p_184394_1_, p_184394_2_);
            GlStateManager.cullFace(GlStateManager.CullFace.BACK);
            GlStateManager.popMatrix();
            GlStateManager.disableRescaleNormal();
            GlStateManager.disableBlend();
            this.textureManager.bindTexture(TextureMap.locationBlocksTexture);
            this.textureManager.getTexture(TextureMap.locationBlocksTexture).restoreLastBlurMipmap();
        }
    }

    /**
     * Return true if only one scale is negative
     */
    private boolean isThereOneNegativeScale(ItemTransformVec3f itemTranformVec)
    {
        return itemTranformVec.scale.x < 0.0F ^ itemTranformVec.scale.y < 0.0F ^ itemTranformVec.scale.z < 0.0F;
    }

    public void renderItemIntoGUI(ItemStack stack, int x, int y)
    {
        this.func_184390_a(stack, x, y, this.getItemModelWithOverrides(stack, (World)null, (EntityLivingBase)null));
    }

    protected void func_184390_a(ItemStack p_184390_1_, int p_184390_2_, int p_184390_3_, IBakedModel p_184390_4_)
    {
        GlStateManager.pushMatrix();
        this.textureManager.bindTexture(TextureMap.locationBlocksTexture);
        this.textureManager.getTexture(TextureMap.locationBlocksTexture).setBlurMipmap(false, false);
        GlStateManager.enableRescaleNormal();
        GlStateManager.enableAlpha();
        GlStateManager.alphaFunc(516, 0.1F);
        GlStateManager.enableBlend();
        GlStateManager.blendFunc(GlStateManager.SourceFactor.SRC_ALPHA, GlStateManager.DestFactor.ONE_MINUS_SRC_ALPHA);
        GlStateManager.color(1.0F, 1.0F, 1.0F, 1.0F);
        this.setupGuiTransform(p_184390_2_, p_184390_3_, p_184390_4_.isGui3d());
        p_184390_4_.getItemCameraTransforms().applyTransform(ItemCameraTransforms.TransformType.GUI);
        this.renderItem(p_184390_1_, p_184390_4_);
        GlStateManager.disableAlpha();
        GlStateManager.disableRescaleNormal();
        GlStateManager.disableLighting();
        GlStateManager.popMatrix();
        this.textureManager.bindTexture(TextureMap.locationBlocksTexture);
        this.textureManager.getTexture(TextureMap.locationBlocksTexture).restoreLastBlurMipmap();
    }

    private void setupGuiTransform(int xPosition, int yPosition, boolean isGui3d)
    {
        GlStateManager.translate((float)xPosition, (float)yPosition, 100.0F + this.zLevel);
        GlStateManager.translate(8.0F, 8.0F, 0.0F);
        GlStateManager.scale(1.0F, -1.0F, 1.0F);
        GlStateManager.scale(16.0F, 16.0F, 16.0F);

        if (isGui3d)
        {
            GlStateManager.enableLighting();
        }
        else
        {
            GlStateManager.disableLighting();
        }
    }

    public void renderItemAndEffectIntoGUI(ItemStack stack, int xPosition, int yPosition)
    {
        this.func_184391_a(Minecraft.getMinecraft().thePlayer, stack, xPosition, yPosition);
    }

    public void func_184391_a(EntityLivingBase p_184391_1_, final ItemStack p_184391_2_, int p_184391_3_, int p_184391_4_)
    {
        if (p_184391_2_ != null && p_184391_2_.getItem() != null)
        {
            this.zLevel += 50.0F;

            try
            {
                this.func_184390_a(p_184391_2_, p_184391_3_, p_184391_4_, this.getItemModelWithOverrides(p_184391_2_, (World)null, p_184391_1_));
            }
            catch (Throwable throwable)
            {
                CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Rendering item");
                CrashReportCategory crashreportcategory = crashreport.makeCategory("Item being rendered");
                crashreportcategory.addCrashSectionCallable("Item Type", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return String.valueOf((Object)p_184391_2_.getItem());
                    }
                });
                crashreportcategory.addCrashSectionCallable("Item Aux", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return String.valueOf(p_184391_2_.getMetadata());
                    }
                });
                crashreportcategory.addCrashSectionCallable("Item NBT", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return String.valueOf((Object)p_184391_2_.getTagCompound());
                    }
                });
                crashreportcategory.addCrashSectionCallable("Item Foil", new Callable<String>()
                {
                    public String call() throws Exception
                    {
                        return String.valueOf(p_184391_2_.hasEffect());
                    }
                });
                throw new ReportedException(crashreport);
            }

            this.zLevel -= 50.0F;
        }
    }

    public void renderItemOverlays(FontRenderer fr, ItemStack stack, int xPosition, int yPosition)
    {
        this.renderItemOverlayIntoGUI(fr, stack, xPosition, yPosition, (String)null);
    }

    /**
     * Renders the stack size and/or damage bar for the given ItemStack.
     */
    public void renderItemOverlayIntoGUI(FontRenderer fr, ItemStack stack, int xPosition, int yPosition, String text)
    {
        if (stack != null)
        {
            if (stack.stackSize != 1 || text != null)
            {
                String s = text == null ? String.valueOf(stack.stackSize) : text;

                if (text == null && stack.stackSize < 1)
                {
                    s = TextFormatting.RED + String.valueOf(stack.stackSize);
                }

                GlStateManager.disableLighting();
                GlStateManager.disableDepth();
                GlStateManager.disableBlend();
                fr.drawStringWithShadow(s, (float)(xPosition + 19 - 2 - fr.getStringWidth(s)), (float)(yPosition + 6 + 3), 16777215);
                GlStateManager.enableLighting();
                GlStateManager.enableDepth();
            }

            if (stack.isItemDamaged())
            {
                int j = (int)Math.round(13.0D - (double)stack.getItemDamage() * 13.0D / (double)stack.getMaxDamage());
                int i = (int)Math.round(255.0D - (double)stack.getItemDamage() * 255.0D / (double)stack.getMaxDamage());
                GlStateManager.disableLighting();
                GlStateManager.disableDepth();
                GlStateManager.disableTexture2D();
                GlStateManager.disableAlpha();
                GlStateManager.disableBlend();
                Tessellator tessellator = Tessellator.getInstance();
                VertexBuffer vertexbuffer = tessellator.getBuffer();
                this.draw(vertexbuffer, xPosition + 2, yPosition + 13, 13, 2, 0, 0, 0, 255);
                this.draw(vertexbuffer, xPosition + 2, yPosition + 13, 12, 1, (255 - i) / 4, 64, 0, 255);
                this.draw(vertexbuffer, xPosition + 2, yPosition + 13, j, 1, 255 - i, i, 0, 255);
                GlStateManager.enableBlend();
                GlStateManager.enableAlpha();
                GlStateManager.enableTexture2D();
                GlStateManager.enableLighting();
                GlStateManager.enableDepth();
            }

            EntityPlayerSP entityplayersp = Minecraft.getMinecraft().thePlayer;
            float f = entityplayersp == null ? 0.0F : entityplayersp.func_184811_cZ().getCooldown(stack.getItem(), Minecraft.getMinecraft().func_184121_ak());

            if (f > 0.0F)
            {
                GlStateManager.disableLighting();
                GlStateManager.disableDepth();
                GlStateManager.disableTexture2D();
                Tessellator tessellator1 = Tessellator.getInstance();
                VertexBuffer vertexbuffer1 = tessellator1.getBuffer();
                this.draw(vertexbuffer1, xPosition, yPosition + MathHelper.floor_float(16.0F * (1.0F - f)), 16, MathHelper.ceiling_float_int(16.0F * f), 255, 255, 255, 127);
                GlStateManager.enableTexture2D();
                GlStateManager.enableLighting();
                GlStateManager.enableDepth();
            }
        }
    }

    /**
     * Draw with the WorldRenderer
     */
    private void draw(VertexBuffer renderer, int x, int y, int width, int height, int red, int green, int blue, int alpha)
    {
        renderer.begin(7, DefaultVertexFormats.POSITION_COLOR);
        renderer.pos((double)(x + 0), (double)(y + 0), 0.0D).color(red, green, blue, alpha).endVertex();
        renderer.pos((double)(x + 0), (double)(y + height), 0.0D).color(red, green, blue, alpha).endVertex();
        renderer.pos((double)(x + width), (double)(y + height), 0.0D).color(red, green, blue, alpha).endVertex();
        renderer.pos((double)(x + width), (double)(y + 0), 0.0D).color(red, green, blue, alpha).endVertex();
        Tessellator.getInstance().draw();
    }

    private void registerItems()
    {
        this.registerBlock(Blocks.anvil, "anvil_intact");
        this.registerBlock(Blocks.anvil, 1, "anvil_slightly_damaged");
        this.registerBlock(Blocks.anvil, 2, "anvil_very_damaged");
        this.registerBlock(Blocks.carpet, EnumDyeColor.BLACK.getMetadata(), "black_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.BLUE.getMetadata(), "blue_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.BROWN.getMetadata(), "brown_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.CYAN.getMetadata(), "cyan_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.GRAY.getMetadata(), "gray_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.GREEN.getMetadata(), "green_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.LIGHT_BLUE.getMetadata(), "light_blue_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.LIME.getMetadata(), "lime_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.MAGENTA.getMetadata(), "magenta_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.ORANGE.getMetadata(), "orange_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.PINK.getMetadata(), "pink_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.PURPLE.getMetadata(), "purple_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.RED.getMetadata(), "red_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.SILVER.getMetadata(), "silver_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.WHITE.getMetadata(), "white_carpet");
        this.registerBlock(Blocks.carpet, EnumDyeColor.YELLOW.getMetadata(), "yellow_carpet");
        this.registerBlock(Blocks.cobblestone_wall, BlockWall.EnumType.MOSSY.getMetadata(), "mossy_cobblestone_wall");
        this.registerBlock(Blocks.cobblestone_wall, BlockWall.EnumType.NORMAL.getMetadata(), "cobblestone_wall");
        this.registerBlock(Blocks.dirt, BlockDirt.DirtType.COARSE_DIRT.getMetadata(), "coarse_dirt");
        this.registerBlock(Blocks.dirt, BlockDirt.DirtType.DIRT.getMetadata(), "dirt");
        this.registerBlock(Blocks.dirt, BlockDirt.DirtType.PODZOL.getMetadata(), "podzol");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.FERN.getMeta(), "double_fern");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.GRASS.getMeta(), "double_grass");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.PAEONIA.getMeta(), "paeonia");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.ROSE.getMeta(), "double_rose");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.SUNFLOWER.getMeta(), "sunflower");
        this.registerBlock(Blocks.double_plant, BlockDoublePlant.EnumPlantType.SYRINGA.getMeta(), "syringa");
        this.registerBlock(Blocks.leaves, BlockPlanks.EnumType.BIRCH.getMetadata(), "birch_leaves");
        this.registerBlock(Blocks.leaves, BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle_leaves");
        this.registerBlock(Blocks.leaves, BlockPlanks.EnumType.OAK.getMetadata(), "oak_leaves");
        this.registerBlock(Blocks.leaves, BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce_leaves");
        this.registerBlock(Blocks.leaves2, BlockPlanks.EnumType.ACACIA.getMetadata() - 4, "acacia_leaves");
        this.registerBlock(Blocks.leaves2, BlockPlanks.EnumType.DARK_OAK.getMetadata() - 4, "dark_oak_leaves");
        this.registerBlock(Blocks.log, BlockPlanks.EnumType.BIRCH.getMetadata(), "birch_log");
        this.registerBlock(Blocks.log, BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle_log");
        this.registerBlock(Blocks.log, BlockPlanks.EnumType.OAK.getMetadata(), "oak_log");
        this.registerBlock(Blocks.log, BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce_log");
        this.registerBlock(Blocks.log2, BlockPlanks.EnumType.ACACIA.getMetadata() - 4, "acacia_log");
        this.registerBlock(Blocks.log2, BlockPlanks.EnumType.DARK_OAK.getMetadata() - 4, "dark_oak_log");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.CHISELED_STONEBRICK.getMetadata(), "chiseled_brick_monster_egg");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.COBBLESTONE.getMetadata(), "cobblestone_monster_egg");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.CRACKED_STONEBRICK.getMetadata(), "cracked_brick_monster_egg");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.MOSSY_STONEBRICK.getMetadata(), "mossy_brick_monster_egg");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.STONE.getMetadata(), "stone_monster_egg");
        this.registerBlock(Blocks.monster_egg, BlockSilverfish.EnumType.STONEBRICK.getMetadata(), "stone_brick_monster_egg");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.ACACIA.getMetadata(), "acacia_planks");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.BIRCH.getMetadata(), "birch_planks");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.DARK_OAK.getMetadata(), "dark_oak_planks");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle_planks");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.OAK.getMetadata(), "oak_planks");
        this.registerBlock(Blocks.planks, BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce_planks");
        this.registerBlock(Blocks.prismarine, BlockPrismarine.EnumType.BRICKS.getMetadata(), "prismarine_bricks");
        this.registerBlock(Blocks.prismarine, BlockPrismarine.EnumType.DARK.getMetadata(), "dark_prismarine");
        this.registerBlock(Blocks.prismarine, BlockPrismarine.EnumType.ROUGH.getMetadata(), "prismarine");
        this.registerBlock(Blocks.quartz_block, BlockQuartz.EnumType.CHISELED.getMetadata(), "chiseled_quartz_block");
        this.registerBlock(Blocks.quartz_block, BlockQuartz.EnumType.DEFAULT.getMetadata(), "quartz_block");
        this.registerBlock(Blocks.quartz_block, BlockQuartz.EnumType.LINES_Y.getMetadata(), "quartz_column");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.ALLIUM.getMeta(), "allium");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.BLUE_ORCHID.getMeta(), "blue_orchid");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.HOUSTONIA.getMeta(), "houstonia");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.ORANGE_TULIP.getMeta(), "orange_tulip");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.OXEYE_DAISY.getMeta(), "oxeye_daisy");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.PINK_TULIP.getMeta(), "pink_tulip");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.POPPY.getMeta(), "poppy");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.RED_TULIP.getMeta(), "red_tulip");
        this.registerBlock(Blocks.red_flower, BlockFlower.EnumFlowerType.WHITE_TULIP.getMeta(), "white_tulip");
        this.registerBlock(Blocks.sand, BlockSand.EnumType.RED_SAND.getMetadata(), "red_sand");
        this.registerBlock(Blocks.sand, BlockSand.EnumType.SAND.getMetadata(), "sand");
        this.registerBlock(Blocks.sandstone, BlockSandStone.EnumType.CHISELED.getMetadata(), "chiseled_sandstone");
        this.registerBlock(Blocks.sandstone, BlockSandStone.EnumType.DEFAULT.getMetadata(), "sandstone");
        this.registerBlock(Blocks.sandstone, BlockSandStone.EnumType.SMOOTH.getMetadata(), "smooth_sandstone");
        this.registerBlock(Blocks.red_sandstone, BlockRedSandstone.EnumType.CHISELED.getMetadata(), "chiseled_red_sandstone");
        this.registerBlock(Blocks.red_sandstone, BlockRedSandstone.EnumType.DEFAULT.getMetadata(), "red_sandstone");
        this.registerBlock(Blocks.red_sandstone, BlockRedSandstone.EnumType.SMOOTH.getMetadata(), "smooth_red_sandstone");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.ACACIA.getMetadata(), "acacia_sapling");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.BIRCH.getMetadata(), "birch_sapling");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.DARK_OAK.getMetadata(), "dark_oak_sapling");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle_sapling");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.OAK.getMetadata(), "oak_sapling");
        this.registerBlock(Blocks.sapling, BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce_sapling");
        this.registerBlock(Blocks.sponge, 0, "sponge");
        this.registerBlock(Blocks.sponge, 1, "sponge_wet");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.BLACK.getMetadata(), "black_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.BLUE.getMetadata(), "blue_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.BROWN.getMetadata(), "brown_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.CYAN.getMetadata(), "cyan_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.GRAY.getMetadata(), "gray_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.GREEN.getMetadata(), "green_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.LIGHT_BLUE.getMetadata(), "light_blue_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.LIME.getMetadata(), "lime_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.MAGENTA.getMetadata(), "magenta_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.ORANGE.getMetadata(), "orange_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.PINK.getMetadata(), "pink_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.PURPLE.getMetadata(), "purple_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.RED.getMetadata(), "red_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.SILVER.getMetadata(), "silver_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.WHITE.getMetadata(), "white_stained_glass");
        this.registerBlock(Blocks.stained_glass, EnumDyeColor.YELLOW.getMetadata(), "yellow_stained_glass");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.BLACK.getMetadata(), "black_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.BLUE.getMetadata(), "blue_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.BROWN.getMetadata(), "brown_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.CYAN.getMetadata(), "cyan_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.GRAY.getMetadata(), "gray_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.GREEN.getMetadata(), "green_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.LIGHT_BLUE.getMetadata(), "light_blue_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.LIME.getMetadata(), "lime_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.MAGENTA.getMetadata(), "magenta_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.ORANGE.getMetadata(), "orange_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.PINK.getMetadata(), "pink_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.PURPLE.getMetadata(), "purple_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.RED.getMetadata(), "red_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.SILVER.getMetadata(), "silver_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.WHITE.getMetadata(), "white_stained_glass_pane");
        this.registerBlock(Blocks.stained_glass_pane, EnumDyeColor.YELLOW.getMetadata(), "yellow_stained_glass_pane");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.BLACK.getMetadata(), "black_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.BLUE.getMetadata(), "blue_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.BROWN.getMetadata(), "brown_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.CYAN.getMetadata(), "cyan_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.GRAY.getMetadata(), "gray_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.GREEN.getMetadata(), "green_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.LIGHT_BLUE.getMetadata(), "light_blue_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.LIME.getMetadata(), "lime_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.MAGENTA.getMetadata(), "magenta_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.ORANGE.getMetadata(), "orange_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.PINK.getMetadata(), "pink_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.PURPLE.getMetadata(), "purple_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.RED.getMetadata(), "red_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.SILVER.getMetadata(), "silver_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.WHITE.getMetadata(), "white_stained_hardened_clay");
        this.registerBlock(Blocks.stained_hardened_clay, EnumDyeColor.YELLOW.getMetadata(), "yellow_stained_hardened_clay");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.ANDESITE.getMetadata(), "andesite");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.ANDESITE_SMOOTH.getMetadata(), "andesite_smooth");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.DIORITE.getMetadata(), "diorite");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.DIORITE_SMOOTH.getMetadata(), "diorite_smooth");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.GRANITE.getMetadata(), "granite");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.GRANITE_SMOOTH.getMetadata(), "granite_smooth");
        this.registerBlock(Blocks.stone, BlockStone.EnumType.STONE.getMetadata(), "stone");
        this.registerBlock(Blocks.stonebrick, BlockStoneBrick.EnumType.CRACKED.getMetadata(), "cracked_stonebrick");
        this.registerBlock(Blocks.stonebrick, BlockStoneBrick.EnumType.DEFAULT.getMetadata(), "stonebrick");
        this.registerBlock(Blocks.stonebrick, BlockStoneBrick.EnumType.CHISELED.getMetadata(), "chiseled_stonebrick");
        this.registerBlock(Blocks.stonebrick, BlockStoneBrick.EnumType.MOSSY.getMetadata(), "mossy_stonebrick");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.BRICK.getMetadata(), "brick_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.COBBLESTONE.getMetadata(), "cobblestone_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.WOOD.getMetadata(), "old_wood_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.NETHERBRICK.getMetadata(), "nether_brick_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.QUARTZ.getMetadata(), "quartz_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.SAND.getMetadata(), "sandstone_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.SMOOTHBRICK.getMetadata(), "stone_brick_slab");
        this.registerBlock(Blocks.stone_slab, BlockStoneSlab.EnumType.STONE.getMetadata(), "stone_slab");
        this.registerBlock(Blocks.stone_slab2, BlockStoneSlabNew.EnumType.RED_SANDSTONE.getMetadata(), "red_sandstone_slab");
        this.registerBlock(Blocks.tallgrass, BlockTallGrass.EnumType.DEAD_BUSH.getMeta(), "dead_bush");
        this.registerBlock(Blocks.tallgrass, BlockTallGrass.EnumType.FERN.getMeta(), "fern");
        this.registerBlock(Blocks.tallgrass, BlockTallGrass.EnumType.GRASS.getMeta(), "tall_grass");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.ACACIA.getMetadata(), "acacia_slab");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.BIRCH.getMetadata(), "birch_slab");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.DARK_OAK.getMetadata(), "dark_oak_slab");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.JUNGLE.getMetadata(), "jungle_slab");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.OAK.getMetadata(), "oak_slab");
        this.registerBlock(Blocks.wooden_slab, BlockPlanks.EnumType.SPRUCE.getMetadata(), "spruce_slab");
        this.registerBlock(Blocks.wool, EnumDyeColor.BLACK.getMetadata(), "black_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.BLUE.getMetadata(), "blue_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.BROWN.getMetadata(), "brown_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.CYAN.getMetadata(), "cyan_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.GRAY.getMetadata(), "gray_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.GREEN.getMetadata(), "green_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.LIGHT_BLUE.getMetadata(), "light_blue_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.LIME.getMetadata(), "lime_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.MAGENTA.getMetadata(), "magenta_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.ORANGE.getMetadata(), "orange_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.PINK.getMetadata(), "pink_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.PURPLE.getMetadata(), "purple_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.RED.getMetadata(), "red_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.SILVER.getMetadata(), "silver_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.WHITE.getMetadata(), "white_wool");
        this.registerBlock(Blocks.wool, EnumDyeColor.YELLOW.getMetadata(), "yellow_wool");
        this.registerBlock(Blocks.farmland, "farmland");
        this.registerBlock(Blocks.acacia_stairs, "acacia_stairs");
        this.registerBlock(Blocks.activator_rail, "activator_rail");
        this.registerBlock(Blocks.beacon, "beacon");
        this.registerBlock(Blocks.bedrock, "bedrock");
        this.registerBlock(Blocks.birch_stairs, "birch_stairs");
        this.registerBlock(Blocks.bookshelf, "bookshelf");
        this.registerBlock(Blocks.brick_block, "brick_block");
        this.registerBlock(Blocks.brick_block, "brick_block");
        this.registerBlock(Blocks.brick_stairs, "brick_stairs");
        this.registerBlock(Blocks.brown_mushroom, "brown_mushroom");
        this.registerBlock(Blocks.cactus, "cactus");
        this.registerBlock(Blocks.clay, "clay");
        this.registerBlock(Blocks.coal_block, "coal_block");
        this.registerBlock(Blocks.coal_ore, "coal_ore");
        this.registerBlock(Blocks.cobblestone, "cobblestone");
        this.registerBlock(Blocks.crafting_table, "crafting_table");
        this.registerBlock(Blocks.dark_oak_stairs, "dark_oak_stairs");
        this.registerBlock(Blocks.daylight_detector, "daylight_detector");
        this.registerBlock(Blocks.deadbush, "dead_bush");
        this.registerBlock(Blocks.detector_rail, "detector_rail");
        this.registerBlock(Blocks.diamond_block, "diamond_block");
        this.registerBlock(Blocks.diamond_ore, "diamond_ore");
        this.registerBlock(Blocks.dispenser, "dispenser");
        this.registerBlock(Blocks.dropper, "dropper");
        this.registerBlock(Blocks.emerald_block, "emerald_block");
        this.registerBlock(Blocks.emerald_ore, "emerald_ore");
        this.registerBlock(Blocks.enchanting_table, "enchanting_table");
        this.registerBlock(Blocks.end_portal_frame, "end_portal_frame");
        this.registerBlock(Blocks.end_stone, "end_stone");
        this.registerBlock(Blocks.oak_fence, "oak_fence");
        this.registerBlock(Blocks.spruce_fence, "spruce_fence");
        this.registerBlock(Blocks.birch_fence, "birch_fence");
        this.registerBlock(Blocks.jungle_fence, "jungle_fence");
        this.registerBlock(Blocks.dark_oak_fence, "dark_oak_fence");
        this.registerBlock(Blocks.acacia_fence, "acacia_fence");
        this.registerBlock(Blocks.oak_fence_gate, "oak_fence_gate");
        this.registerBlock(Blocks.spruce_fence_gate, "spruce_fence_gate");
        this.registerBlock(Blocks.birch_fence_gate, "birch_fence_gate");
        this.registerBlock(Blocks.jungle_fence_gate, "jungle_fence_gate");
        this.registerBlock(Blocks.dark_oak_fence_gate, "dark_oak_fence_gate");
        this.registerBlock(Blocks.acacia_fence_gate, "acacia_fence_gate");
        this.registerBlock(Blocks.furnace, "furnace");
        this.registerBlock(Blocks.glass, "glass");
        this.registerBlock(Blocks.glass_pane, "glass_pane");
        this.registerBlock(Blocks.glowstone, "glowstone");
        this.registerBlock(Blocks.golden_rail, "golden_rail");
        this.registerBlock(Blocks.gold_block, "gold_block");
        this.registerBlock(Blocks.gold_ore, "gold_ore");
        this.registerBlock(Blocks.grass, "grass");
        this.registerBlock(Blocks.grass_path, "grass_path");
        this.registerBlock(Blocks.gravel, "gravel");
        this.registerBlock(Blocks.hardened_clay, "hardened_clay");
        this.registerBlock(Blocks.hay_block, "hay_block");
        this.registerBlock(Blocks.heavy_weighted_pressure_plate, "heavy_weighted_pressure_plate");
        this.registerBlock(Blocks.hopper, "hopper");
        this.registerBlock(Blocks.ice, "ice");
        this.registerBlock(Blocks.iron_bars, "iron_bars");
        this.registerBlock(Blocks.iron_block, "iron_block");
        this.registerBlock(Blocks.iron_ore, "iron_ore");
        this.registerBlock(Blocks.iron_trapdoor, "iron_trapdoor");
        this.registerBlock(Blocks.jukebox, "jukebox");
        this.registerBlock(Blocks.jungle_stairs, "jungle_stairs");
        this.registerBlock(Blocks.ladder, "ladder");
        this.registerBlock(Blocks.lapis_block, "lapis_block");
        this.registerBlock(Blocks.lapis_ore, "lapis_ore");
        this.registerBlock(Blocks.lever, "lever");
        this.registerBlock(Blocks.light_weighted_pressure_plate, "light_weighted_pressure_plate");
        this.registerBlock(Blocks.lit_pumpkin, "lit_pumpkin");
        this.registerBlock(Blocks.melon_block, "melon_block");
        this.registerBlock(Blocks.mossy_cobblestone, "mossy_cobblestone");
        this.registerBlock(Blocks.mycelium, "mycelium");
        this.registerBlock(Blocks.netherrack, "netherrack");
        this.registerBlock(Blocks.nether_brick, "nether_brick");
        this.registerBlock(Blocks.nether_brick_fence, "nether_brick_fence");
        this.registerBlock(Blocks.nether_brick_stairs, "nether_brick_stairs");
        this.registerBlock(Blocks.noteblock, "noteblock");
        this.registerBlock(Blocks.oak_stairs, "oak_stairs");
        this.registerBlock(Blocks.obsidian, "obsidian");
        this.registerBlock(Blocks.packed_ice, "packed_ice");
        this.registerBlock(Blocks.piston, "piston");
        this.registerBlock(Blocks.pumpkin, "pumpkin");
        this.registerBlock(Blocks.quartz_ore, "quartz_ore");
        this.registerBlock(Blocks.quartz_stairs, "quartz_stairs");
        this.registerBlock(Blocks.rail, "rail");
        this.registerBlock(Blocks.redstone_block, "redstone_block");
        this.registerBlock(Blocks.redstone_lamp, "redstone_lamp");
        this.registerBlock(Blocks.redstone_ore, "redstone_ore");
        this.registerBlock(Blocks.redstone_torch, "redstone_torch");
        this.registerBlock(Blocks.red_mushroom, "red_mushroom");
        this.registerBlock(Blocks.sandstone_stairs, "sandstone_stairs");
        this.registerBlock(Blocks.red_sandstone_stairs, "red_sandstone_stairs");
        this.registerBlock(Blocks.sea_lantern, "sea_lantern");
        this.registerBlock(Blocks.slime_block, "slime");
        this.registerBlock(Blocks.snow, "snow");
        this.registerBlock(Blocks.snow_layer, "snow_layer");
        this.registerBlock(Blocks.soul_sand, "soul_sand");
        this.registerBlock(Blocks.spruce_stairs, "spruce_stairs");
        this.registerBlock(Blocks.sticky_piston, "sticky_piston");
        this.registerBlock(Blocks.stone_brick_stairs, "stone_brick_stairs");
        this.registerBlock(Blocks.stone_button, "stone_button");
        this.registerBlock(Blocks.stone_pressure_plate, "stone_pressure_plate");
        this.registerBlock(Blocks.stone_stairs, "stone_stairs");
        this.registerBlock(Blocks.tnt, "tnt");
        this.registerBlock(Blocks.torch, "torch");
        this.registerBlock(Blocks.trapdoor, "trapdoor");
        this.registerBlock(Blocks.tripwire_hook, "tripwire_hook");
        this.registerBlock(Blocks.vine, "vine");
        this.registerBlock(Blocks.waterlily, "waterlily");
        this.registerBlock(Blocks.web, "web");
        this.registerBlock(Blocks.wooden_button, "wooden_button");
        this.registerBlock(Blocks.wooden_pressure_plate, "wooden_pressure_plate");
        this.registerBlock(Blocks.yellow_flower, BlockFlower.EnumFlowerType.DANDELION.getMeta(), "dandelion");
        this.registerBlock(Blocks.end_rod, "end_rod");
        this.registerBlock(Blocks.chorus_plant, "chorus_plant");
        this.registerBlock(Blocks.chorus_flower, "chorus_flower");
        this.registerBlock(Blocks.purpur_block, "purpur_block");
        this.registerBlock(Blocks.purpur_pillar, "purpur_pillar");
        this.registerBlock(Blocks.purpur_stairs, "purpur_stairs");
        this.registerBlock(Blocks.purpur_slab, "purpur_slab");
        this.registerBlock(Blocks.purpur_double_slab, "purpur_double_slab");
        this.registerBlock(Blocks.end_bricks, "end_bricks");
        this.registerBlock(Blocks.chest, "chest");
        this.registerBlock(Blocks.trapped_chest, "trapped_chest");
        this.registerBlock(Blocks.ender_chest, "ender_chest");
        this.registerItem(Items.iron_shovel, "iron_shovel");
        this.registerItem(Items.iron_pickaxe, "iron_pickaxe");
        this.registerItem(Items.iron_axe, "iron_axe");
        this.registerItem(Items.flint_and_steel, "flint_and_steel");
        this.registerItem(Items.apple, "apple");
        this.registerItem(Items.bow, "bow");
        this.registerItem(Items.arrow, "arrow");
        this.registerItem(Items.spectral_arrow, "spectral_arrow");
        this.registerItem(Items.tipped_arrow, "tipped_arrow");
        this.registerItem(Items.coal, 0, "coal");
        this.registerItem(Items.coal, 1, "charcoal");
        this.registerItem(Items.diamond, "diamond");
        this.registerItem(Items.iron_ingot, "iron_ingot");
        this.registerItem(Items.gold_ingot, "gold_ingot");
        this.registerItem(Items.iron_sword, "iron_sword");
        this.registerItem(Items.wooden_sword, "wooden_sword");
        this.registerItem(Items.wooden_shovel, "wooden_shovel");
        this.registerItem(Items.wooden_pickaxe, "wooden_pickaxe");
        this.registerItem(Items.wooden_axe, "wooden_axe");
        this.registerItem(Items.stone_sword, "stone_sword");
        this.registerItem(Items.stone_shovel, "stone_shovel");
        this.registerItem(Items.stone_pickaxe, "stone_pickaxe");
        this.registerItem(Items.stone_axe, "stone_axe");
        this.registerItem(Items.diamond_sword, "diamond_sword");
        this.registerItem(Items.diamond_shovel, "diamond_shovel");
        this.registerItem(Items.diamond_pickaxe, "diamond_pickaxe");
        this.registerItem(Items.diamond_axe, "diamond_axe");
        this.registerItem(Items.stick, "stick");
        this.registerItem(Items.bowl, "bowl");
        this.registerItem(Items.mushroom_stew, "mushroom_stew");
        this.registerItem(Items.golden_sword, "golden_sword");
        this.registerItem(Items.golden_shovel, "golden_shovel");
        this.registerItem(Items.golden_pickaxe, "golden_pickaxe");
        this.registerItem(Items.golden_axe, "golden_axe");
        this.registerItem(Items.string, "string");
        this.registerItem(Items.feather, "feather");
        this.registerItem(Items.gunpowder, "gunpowder");
        this.registerItem(Items.wooden_hoe, "wooden_hoe");
        this.registerItem(Items.stone_hoe, "stone_hoe");
        this.registerItem(Items.iron_hoe, "iron_hoe");
        this.registerItem(Items.diamond_hoe, "diamond_hoe");
        this.registerItem(Items.golden_hoe, "golden_hoe");
        this.registerItem(Items.wheat_seeds, "wheat_seeds");
        this.registerItem(Items.wheat, "wheat");
        this.registerItem(Items.bread, "bread");
        this.registerItem(Items.leather_helmet, "leather_helmet");
        this.registerItem(Items.leather_chestplate, "leather_chestplate");
        this.registerItem(Items.leather_leggings, "leather_leggings");
        this.registerItem(Items.leather_boots, "leather_boots");
        this.registerItem(Items.chainmail_helmet, "chainmail_helmet");
        this.registerItem(Items.chainmail_chestplate, "chainmail_chestplate");
        this.registerItem(Items.chainmail_leggings, "chainmail_leggings");
        this.registerItem(Items.chainmail_boots, "chainmail_boots");
        this.registerItem(Items.iron_helmet, "iron_helmet");
        this.registerItem(Items.iron_chestplate, "iron_chestplate");
        this.registerItem(Items.iron_leggings, "iron_leggings");
        this.registerItem(Items.iron_boots, "iron_boots");
        this.registerItem(Items.diamond_helmet, "diamond_helmet");
        this.registerItem(Items.diamond_chestplate, "diamond_chestplate");
        this.registerItem(Items.diamond_leggings, "diamond_leggings");
        this.registerItem(Items.diamond_boots, "diamond_boots");
        this.registerItem(Items.golden_helmet, "golden_helmet");
        this.registerItem(Items.golden_chestplate, "golden_chestplate");
        this.registerItem(Items.golden_leggings, "golden_leggings");
        this.registerItem(Items.golden_boots, "golden_boots");
        this.registerItem(Items.flint, "flint");
        this.registerItem(Items.porkchop, "porkchop");
        this.registerItem(Items.cooked_porkchop, "cooked_porkchop");
        this.registerItem(Items.painting, "painting");
        this.registerItem(Items.golden_apple, "golden_apple");
        this.registerItem(Items.golden_apple, 1, "golden_apple");
        this.registerItem(Items.sign, "sign");
        this.registerItem(Items.oak_door, "oak_door");
        this.registerItem(Items.spruce_door, "spruce_door");
        this.registerItem(Items.birch_door, "birch_door");
        this.registerItem(Items.jungle_door, "jungle_door");
        this.registerItem(Items.acacia_door, "acacia_door");
        this.registerItem(Items.dark_oak_door, "dark_oak_door");
        this.registerItem(Items.bucket, "bucket");
        this.registerItem(Items.water_bucket, "water_bucket");
        this.registerItem(Items.lava_bucket, "lava_bucket");
        this.registerItem(Items.minecart, "minecart");
        this.registerItem(Items.saddle, "saddle");
        this.registerItem(Items.iron_door, "iron_door");
        this.registerItem(Items.redstone, "redstone");
        this.registerItem(Items.snowball, "snowball");
        this.registerItem(Items.boat, "oak_boat");
        this.registerItem(Items.spruce_boat, "spruce_boat");
        this.registerItem(Items.birch_boat, "birch_boat");
        this.registerItem(Items.jungle_boat, "jungle_boat");
        this.registerItem(Items.acacia_boat, "acacia_boat");
        this.registerItem(Items.dark_oak_boat, "dark_oak_boat");
        this.registerItem(Items.leather, "leather");
        this.registerItem(Items.milk_bucket, "milk_bucket");
        this.registerItem(Items.brick, "brick");
        this.registerItem(Items.clay_ball, "clay_ball");
        this.registerItem(Items.reeds, "reeds");
        this.registerItem(Items.paper, "paper");
        this.registerItem(Items.book, "book");
        this.registerItem(Items.slime_ball, "slime_ball");
        this.registerItem(Items.chest_minecart, "chest_minecart");
        this.registerItem(Items.furnace_minecart, "furnace_minecart");
        this.registerItem(Items.egg, "egg");
        this.registerItem(Items.compass, "compass");
        this.registerItem(Items.fishing_rod, "fishing_rod");
        this.registerItem(Items.clock, "clock");
        this.registerItem(Items.glowstone_dust, "glowstone_dust");
        this.registerItem(Items.fish, ItemFishFood.FishType.COD.getMetadata(), "cod");
        this.registerItem(Items.fish, ItemFishFood.FishType.SALMON.getMetadata(), "salmon");
        this.registerItem(Items.fish, ItemFishFood.FishType.CLOWNFISH.getMetadata(), "clownfish");
        this.registerItem(Items.fish, ItemFishFood.FishType.PUFFERFISH.getMetadata(), "pufferfish");
        this.registerItem(Items.cooked_fish, ItemFishFood.FishType.COD.getMetadata(), "cooked_cod");
        this.registerItem(Items.cooked_fish, ItemFishFood.FishType.SALMON.getMetadata(), "cooked_salmon");
        this.registerItem(Items.dye, EnumDyeColor.BLACK.getDyeDamage(), "dye_black");
        this.registerItem(Items.dye, EnumDyeColor.RED.getDyeDamage(), "dye_red");
        this.registerItem(Items.dye, EnumDyeColor.GREEN.getDyeDamage(), "dye_green");
        this.registerItem(Items.dye, EnumDyeColor.BROWN.getDyeDamage(), "dye_brown");
        this.registerItem(Items.dye, EnumDyeColor.BLUE.getDyeDamage(), "dye_blue");
        this.registerItem(Items.dye, EnumDyeColor.PURPLE.getDyeDamage(), "dye_purple");
        this.registerItem(Items.dye, EnumDyeColor.CYAN.getDyeDamage(), "dye_cyan");
        this.registerItem(Items.dye, EnumDyeColor.SILVER.getDyeDamage(), "dye_silver");
        this.registerItem(Items.dye, EnumDyeColor.GRAY.getDyeDamage(), "dye_gray");
        this.registerItem(Items.dye, EnumDyeColor.PINK.getDyeDamage(), "dye_pink");
        this.registerItem(Items.dye, EnumDyeColor.LIME.getDyeDamage(), "dye_lime");
        this.registerItem(Items.dye, EnumDyeColor.YELLOW.getDyeDamage(), "dye_yellow");
        this.registerItem(Items.dye, EnumDyeColor.LIGHT_BLUE.getDyeDamage(), "dye_light_blue");
        this.registerItem(Items.dye, EnumDyeColor.MAGENTA.getDyeDamage(), "dye_magenta");
        this.registerItem(Items.dye, EnumDyeColor.ORANGE.getDyeDamage(), "dye_orange");
        this.registerItem(Items.dye, EnumDyeColor.WHITE.getDyeDamage(), "dye_white");
        this.registerItem(Items.bone, "bone");
        this.registerItem(Items.sugar, "sugar");
        this.registerItem(Items.cake, "cake");
        this.registerItem(Items.bed, "bed");
        this.registerItem(Items.repeater, "repeater");
        this.registerItem(Items.cookie, "cookie");
        this.registerItem(Items.shears, "shears");
        this.registerItem(Items.melon, "melon");
        this.registerItem(Items.pumpkin_seeds, "pumpkin_seeds");
        this.registerItem(Items.melon_seeds, "melon_seeds");
        this.registerItem(Items.beef, "beef");
        this.registerItem(Items.cooked_beef, "cooked_beef");
        this.registerItem(Items.chicken, "chicken");
        this.registerItem(Items.cooked_chicken, "cooked_chicken");
        this.registerItem(Items.rabbit, "rabbit");
        this.registerItem(Items.cooked_rabbit, "cooked_rabbit");
        this.registerItem(Items.mutton, "mutton");
        this.registerItem(Items.cooked_mutton, "cooked_mutton");
        this.registerItem(Items.rabbit_foot, "rabbit_foot");
        this.registerItem(Items.rabbit_hide, "rabbit_hide");
        this.registerItem(Items.rabbit_stew, "rabbit_stew");
        this.registerItem(Items.rotten_flesh, "rotten_flesh");
        this.registerItem(Items.ender_pearl, "ender_pearl");
        this.registerItem(Items.blaze_rod, "blaze_rod");
        this.registerItem(Items.ghast_tear, "ghast_tear");
        this.registerItem(Items.gold_nugget, "gold_nugget");
        this.registerItem(Items.nether_wart, "nether_wart");
        this.registerItem(Items.beetroot, "beetroot");
        this.registerItem(Items.beetroot_seeds, "beetroot_seeds");
        this.registerItem(Items.beetroot_soup, "beetroot_soup");
        this.registerItem(Items.potionitem, "bottle_drinkable");
        this.registerItem(Items.splash_potion, "bottle_splash");
        this.registerItem(Items.lingering_potion, "bottle_lingering");
        this.registerItem(Items.glass_bottle, "glass_bottle");
        this.registerItem(Items.dragon_breath, "dragon_breath");
        this.registerItem(Items.spider_eye, "spider_eye");
        this.registerItem(Items.fermented_spider_eye, "fermented_spider_eye");
        this.registerItem(Items.blaze_powder, "blaze_powder");
        this.registerItem(Items.magma_cream, "magma_cream");
        this.registerItem(Items.brewing_stand, "brewing_stand");
        this.registerItem(Items.cauldron, "cauldron");
        this.registerItem(Items.ender_eye, "ender_eye");
        this.registerItem(Items.speckled_melon, "speckled_melon");
        this.itemModelMesher.register(Items.spawn_egg, new ItemMeshDefinition()
        {
            public ModelResourceLocation getModelLocation(ItemStack stack)
            {
                return new ModelResourceLocation("spawn_egg", "inventory");
            }
        });
        this.registerItem(Items.experience_bottle, "experience_bottle");
        this.registerItem(Items.fire_charge, "fire_charge");
        this.registerItem(Items.writable_book, "writable_book");
        this.registerItem(Items.emerald, "emerald");
        this.registerItem(Items.item_frame, "item_frame");
        this.registerItem(Items.flower_pot, "flower_pot");
        this.registerItem(Items.carrot, "carrot");
        this.registerItem(Items.potato, "potato");
        this.registerItem(Items.baked_potato, "baked_potato");
        this.registerItem(Items.poisonous_potato, "poisonous_potato");
        this.registerItem(Items.map, "map");
        this.registerItem(Items.golden_carrot, "golden_carrot");
        this.registerItem(Items.skull, 0, "skull_skeleton");
        this.registerItem(Items.skull, 1, "skull_wither");
        this.registerItem(Items.skull, 2, "skull_zombie");
        this.registerItem(Items.skull, 3, "skull_char");
        this.registerItem(Items.skull, 4, "skull_creeper");
        this.registerItem(Items.skull, 5, "skull_dragon");
        this.registerItem(Items.carrot_on_a_stick, "carrot_on_a_stick");
        this.registerItem(Items.nether_star, "nether_star");
        this.registerItem(Items.end_crystal, "end_crystal");
        this.registerItem(Items.pumpkin_pie, "pumpkin_pie");
        this.registerItem(Items.firework_charge, "firework_charge");
        this.registerItem(Items.comparator, "comparator");
        this.registerItem(Items.netherbrick, "netherbrick");
        this.registerItem(Items.quartz, "quartz");
        this.registerItem(Items.tnt_minecart, "tnt_minecart");
        this.registerItem(Items.hopper_minecart, "hopper_minecart");
        this.registerItem(Items.armor_stand, "armor_stand");
        this.registerItem(Items.iron_horse_armor, "iron_horse_armor");
        this.registerItem(Items.golden_horse_armor, "golden_horse_armor");
        this.registerItem(Items.diamond_horse_armor, "diamond_horse_armor");
        this.registerItem(Items.lead, "lead");
        this.registerItem(Items.name_tag, "name_tag");
        this.itemModelMesher.register(Items.banner, new ItemMeshDefinition()
        {
            public ModelResourceLocation getModelLocation(ItemStack stack)
            {
                return new ModelResourceLocation("banner", "inventory");
            }
        });
        this.itemModelMesher.register(Items.shield, new ItemMeshDefinition()
        {
            public ModelResourceLocation getModelLocation(ItemStack stack)
            {
                return new ModelResourceLocation("shield", "inventory");
            }
        });
        this.registerItem(Items.elytra, "elytra");
        this.registerItem(Items.chorus_fruit, "chorus_fruit");
        this.registerItem(Items.chorus_fruit_popped, "chorus_fruit_popped");
        this.registerItem(Items.record_13, "record_13");
        this.registerItem(Items.record_cat, "record_cat");
        this.registerItem(Items.record_blocks, "record_blocks");
        this.registerItem(Items.record_chirp, "record_chirp");
        this.registerItem(Items.record_far, "record_far");
        this.registerItem(Items.record_mall, "record_mall");
        this.registerItem(Items.record_mellohi, "record_mellohi");
        this.registerItem(Items.record_stal, "record_stal");
        this.registerItem(Items.record_strad, "record_strad");
        this.registerItem(Items.record_ward, "record_ward");
        this.registerItem(Items.record_11, "record_11");
        this.registerItem(Items.record_wait, "record_wait");
        this.registerItem(Items.prismarine_shard, "prismarine_shard");
        this.registerItem(Items.prismarine_crystals, "prismarine_crystals");
        this.itemModelMesher.register(Items.enchanted_book, new ItemMeshDefinition()
        {
            public ModelResourceLocation getModelLocation(ItemStack stack)
            {
                return new ModelResourceLocation("enchanted_book", "inventory");
            }
        });
        this.itemModelMesher.register(Items.filled_map, new ItemMeshDefinition()
        {
            public ModelResourceLocation getModelLocation(ItemStack stack)
            {
                return new ModelResourceLocation("filled_map", "inventory");
            }
        });
        this.registerBlock(Blocks.command_block, "command_block");
        this.registerItem(Items.fireworks, "fireworks");
        this.registerItem(Items.command_block_minecart, "command_block_minecart");
        this.registerBlock(Blocks.barrier, "barrier");
        this.registerBlock(Blocks.mob_spawner, "mob_spawner");
        this.registerItem(Items.written_book, "written_book");
        this.registerBlock(Blocks.brown_mushroom_block, BlockHugeMushroom.EnumType.ALL_INSIDE.getMetadata(), "brown_mushroom_block");
        this.registerBlock(Blocks.red_mushroom_block, BlockHugeMushroom.EnumType.ALL_INSIDE.getMetadata(), "red_mushroom_block");
        this.registerBlock(Blocks.dragon_egg, "dragon_egg");
        this.registerBlock(Blocks.repeating_command_block, "repeating_command_block");
        this.registerBlock(Blocks.chain_command_block, "chain_command_block");
        this.registerBlock(Blocks.structure_block, TileEntityStructure.Mode.SAVE.func_185110_a(), "structure_block");
        this.registerBlock(Blocks.structure_block, TileEntityStructure.Mode.LOAD.func_185110_a(), "structure_block");
        this.registerBlock(Blocks.structure_block, TileEntityStructure.Mode.CORNER.func_185110_a(), "structure_block");
        this.registerBlock(Blocks.structure_block, TileEntityStructure.Mode.DATA.func_185110_a(), "structure_block");
    }

    public void onResourceManagerReload(IResourceManager resourceManager)
    {
        this.itemModelMesher.rebuildCache();
    }
}
