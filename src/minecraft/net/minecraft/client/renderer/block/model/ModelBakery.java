package net.minecraft.client.renderer.block.model;

import com.google.common.base.Charsets;
import com.google.common.base.Joiner;
import com.google.common.base.Predicate;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Queues;
import com.google.common.collect.Sets;
import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.Deque;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.BlockModelShapes;
import net.minecraft.client.renderer.block.model.multipart.Multipart;
import net.minecraft.client.renderer.block.model.multipart.Selector;
import net.minecraft.client.renderer.block.statemap.BlockStateMapper;
import net.minecraft.client.renderer.texture.IIconCreator;
import net.minecraft.client.renderer.texture.TextureAtlasSprite;
import net.minecraft.client.renderer.texture.TextureMap;
import net.minecraft.client.resources.IResource;
import net.minecraft.client.resources.IResourceManager;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.Item;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.registry.IRegistry;
import net.minecraft.util.registry.RegistrySimple;
import org.apache.commons.io.IOUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class ModelBakery
{
    private static final Set<ResourceLocation> LOCATIONS_BUILTIN_TEXTURES = Sets.newHashSet(new ResourceLocation[] {new ResourceLocation("blocks/water_flow"), new ResourceLocation("blocks/water_still"), new ResourceLocation("blocks/lava_flow"), new ResourceLocation("blocks/lava_still"), new ResourceLocation("blocks/water_overlay"), new ResourceLocation("blocks/destroy_stage_0"), new ResourceLocation("blocks/destroy_stage_1"), new ResourceLocation("blocks/destroy_stage_2"), new ResourceLocation("blocks/destroy_stage_3"), new ResourceLocation("blocks/destroy_stage_4"), new ResourceLocation("blocks/destroy_stage_5"), new ResourceLocation("blocks/destroy_stage_6"), new ResourceLocation("blocks/destroy_stage_7"), new ResourceLocation("blocks/destroy_stage_8"), new ResourceLocation("blocks/destroy_stage_9"), new ResourceLocation("items/empty_armor_slot_helmet"), new ResourceLocation("items/empty_armor_slot_chestplate"), new ResourceLocation("items/empty_armor_slot_leggings"), new ResourceLocation("items/empty_armor_slot_boots"), new ResourceLocation("items/empty_armor_slot_shield")});
    private static final Logger LOGGER = LogManager.getLogger();
    protected static final ModelResourceLocation MODEL_MISSING = new ModelResourceLocation("builtin/missing", "missing");
    private static final String field_188641_d = "{    \'textures\': {       \'particle\': \'missingno\',       \'missingno\': \'missingno\'    },    \'elements\': [         {  \'from\': [ 0, 0, 0 ],            \'to\': [ 16, 16, 16 ],            \'faces\': {                \'down\':  { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'down\',  \'texture\': \'#missingno\' },                \'up\':    { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'up\',    \'texture\': \'#missingno\' },                \'north\': { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'north\', \'texture\': \'#missingno\' },                \'south\': { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'south\', \'texture\': \'#missingno\' },                \'west\':  { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'west\',  \'texture\': \'#missingno\' },                \'east\':  { \'uv\': [ 0, 0, 16, 16 ], \'cullface\': \'east\',  \'texture\': \'#missingno\' }            }        }    ]}".replaceAll("\'", "\"");
    private static final Map<String, String> BUILT_IN_MODELS = Maps.<String, String>newHashMap();
    private static final Joiner JOINER = Joiner.on(" -> ");
    private final IResourceManager resourceManager;
    private final Map<ResourceLocation, TextureAtlasSprite> sprites = Maps.<ResourceLocation, TextureAtlasSprite>newHashMap();
    private final Map<ResourceLocation, ModelBlock> models = Maps.<ResourceLocation, ModelBlock>newLinkedHashMap();
    private final Map<ModelResourceLocation, VariantList> variants = Maps.<ModelResourceLocation, VariantList>newLinkedHashMap();
    private final Map<ModelBlockDefinition, Collection<ModelResourceLocation>> multipartVariantMap = Maps.<ModelBlockDefinition, Collection<ModelResourceLocation>>newLinkedHashMap();
    private final TextureMap textureMap;
    private final BlockModelShapes blockModelShapes;
    private final FaceBakery faceBakery = new FaceBakery();
    private final ItemModelGenerator itemModelGenerator = new ItemModelGenerator();
    private RegistrySimple<ModelResourceLocation, IBakedModel> bakedRegistry = new RegistrySimple();
    private static final String EMPTY_MODEL_RAW = "{    \'elements\': [        {   \'from\': [0, 0, 0],            \'to\': [16, 16, 16],            \'faces\': {                \'down\': {\'uv\': [0, 0, 16, 16], \'texture\': \'\' }            }        }    ]}".replaceAll("\'", "\"");
    private static final ModelBlock MODEL_GENERATED = ModelBlock.deserialize(EMPTY_MODEL_RAW);
    private static final ModelBlock MODEL_ENTITY = ModelBlock.deserialize(EMPTY_MODEL_RAW);
    private Map<String, ResourceLocation> itemLocations = Maps.<String, ResourceLocation>newLinkedHashMap();
    private final Map<ResourceLocation, ModelBlockDefinition> blockDefinitions = Maps.<ResourceLocation, ModelBlockDefinition>newHashMap();
    private Map<Item, List<String>> variantNames = Maps.<Item, List<String>>newIdentityHashMap();

    public ModelBakery(IResourceManager p_i46085_1_, TextureMap p_i46085_2_, BlockModelShapes p_i46085_3_)
    {
        this.resourceManager = p_i46085_1_;
        this.textureMap = p_i46085_2_;
        this.blockModelShapes = p_i46085_3_;
    }

    public IRegistry<ModelResourceLocation, IBakedModel> setupModelRegistry()
    {
        this.loadBlocks();
        this.loadVariantItemModels();
        this.loadModelsCheck();
        this.loadSprites();
        this.bakeItemModels();
        this.bakeBlockModels();
        this.func_188635_i();
        return this.bakedRegistry;
    }

    private void loadBlocks()
    {
        BlockStateMapper blockstatemapper = this.blockModelShapes.getBlockStateMapper();

        for (Block block : Block.blockRegistry)
        {
            for (final ResourceLocation resourcelocation : blockstatemapper.getBlockstateLocations(block))
            {
                try
                {
                    ModelBlockDefinition modelblockdefinition = this.getModelBlockDefinition(resourcelocation);
                    Map<IBlockState, ModelResourceLocation> map = blockstatemapper.getVariants(block);

                    if (modelblockdefinition.hasMultipartData())
                    {
                        Collection<ModelResourceLocation> collection = Sets.newHashSet(map.values());
                        modelblockdefinition.getMultipartData().func_188138_a(block.getBlockState());
                        this.multipartVariantMap.put(modelblockdefinition, Lists.newArrayList(Iterables.filter(collection, new Predicate<ModelResourceLocation>()
                        {
                            public boolean apply(ModelResourceLocation p_apply_1_)
                            {
                                return resourcelocation.equals(p_apply_1_);
                            }
                        })));
                    }

                    for (Entry<IBlockState, ModelResourceLocation> entry : map.entrySet())
                    {
                        ModelResourceLocation modelresourcelocation = (ModelResourceLocation)entry.getValue();

                        if (resourcelocation.equals(modelresourcelocation))
                        {
                            try
                            {
                                this.variants.put(modelresourcelocation, modelblockdefinition.func_188004_c(modelresourcelocation.getVariant()));
                            }
                            catch (RuntimeException var12)
                            {
                                if (!modelblockdefinition.hasMultipartData())
                                {
                                    LOGGER.warn("Unable to load variant: " + modelresourcelocation.getVariant() + " from " + modelresourcelocation);
                                }
                            }
                        }
                    }
                }
                catch (Exception exception)
                {
                    LOGGER.warn((String)("Unable to load definition " + resourcelocation), (Throwable)exception);
                }
            }
        }
    }

    private void loadVariantItemModels()
    {
        this.variants.put(MODEL_MISSING, new VariantList(Lists.newArrayList(new Variant[] {new Variant(new ResourceLocation(MODEL_MISSING.getResourcePath()), ModelRotation.X0_Y0, false, 1)})));
        ResourceLocation resourcelocation = new ResourceLocation("item_frame");
        ModelBlockDefinition modelblockdefinition = this.getModelBlockDefinition(resourcelocation);
        this.registerVariant(modelblockdefinition, new ModelResourceLocation(resourcelocation, "normal"));
        this.registerVariant(modelblockdefinition, new ModelResourceLocation(resourcelocation, "map"));
        this.loadVariantModels();
        this.loadMultipartVariantModels();
        this.loadItemModels();
    }

    private void registerVariant(ModelBlockDefinition p_177569_1_, ModelResourceLocation p_177569_2_)
    {
        this.variants.put(p_177569_2_, p_177569_1_.func_188004_c(p_177569_2_.getVariant()));
    }

    private ModelBlockDefinition getModelBlockDefinition(ResourceLocation p_177586_1_)
    {
        ResourceLocation resourcelocation = this.getBlockstateLocation(p_177586_1_);
        ModelBlockDefinition modelblockdefinition = (ModelBlockDefinition)this.blockDefinitions.get(resourcelocation);

        if (modelblockdefinition == null)
        {
            modelblockdefinition = this.loadMultipartMBD(p_177586_1_, resourcelocation);
            this.blockDefinitions.put(resourcelocation, modelblockdefinition);
        }

        return modelblockdefinition;
    }

    private ModelBlockDefinition loadMultipartMBD(ResourceLocation location, ResourceLocation fileIn)
    {
        List<ModelBlockDefinition> list = Lists.<ModelBlockDefinition>newArrayList();

        try
        {
            for (IResource iresource : this.resourceManager.getAllResources(fileIn))
            {
                list.add(this.loadModelBlockDefinition(location, iresource));
            }
        }
        catch (IOException ioexception)
        {
            throw new RuntimeException("Encountered an exception when loading model definition of model " + fileIn.toString(), ioexception);
        }

        return new ModelBlockDefinition(list);
    }

    private ModelBlockDefinition loadModelBlockDefinition(ResourceLocation p_188636_1_, IResource p_188636_2_)
    {
        InputStream inputstream = null;
        ModelBlockDefinition lvt_4_1_;

        try
        {
            inputstream = p_188636_2_.getInputStream();
            lvt_4_1_ = ModelBlockDefinition.parseFromReader(new InputStreamReader(inputstream, Charsets.UTF_8));
        }
        catch (Exception exception)
        {
            throw new RuntimeException("Encountered an exception when loading model definition of \'" + p_188636_1_ + "\' from: \'" + p_188636_2_.getResourceLocation() + "\' in resourcepack: \'" + p_188636_2_.getResourcePackName() + "\'", exception);
        }
        finally
        {
            IOUtils.closeQuietly(inputstream);
        }

        return lvt_4_1_;
    }

    private ResourceLocation getBlockstateLocation(ResourceLocation p_188631_1_)
    {
        return new ResourceLocation(p_188631_1_.getResourceDomain(), "blockstates/" + p_188631_1_.getResourcePath() + ".json");
    }

    private void loadVariantModels()
    {
        for (Entry<ModelResourceLocation, VariantList> entry : this.variants.entrySet())
        {
            this.loadVariantList((ModelResourceLocation)entry.getKey(), (VariantList)entry.getValue());
        }
    }

    private void loadMultipartVariantModels()
    {
        for (Entry<ModelBlockDefinition, Collection<ModelResourceLocation>> entry : this.multipartVariantMap.entrySet())
        {
            ModelResourceLocation modelresourcelocation = (ModelResourceLocation)((Collection)entry.getValue()).iterator().next();

            for (VariantList variantlist : ((ModelBlockDefinition)entry.getKey()).func_188003_a())
            {
                this.loadVariantList(modelresourcelocation, variantlist);
            }
        }
    }

    private void loadVariantList(ModelResourceLocation p_188638_1_, VariantList p_188638_2_)
    {
        for (Variant variant : p_188638_2_.func_188114_a())
        {
            ResourceLocation resourcelocation = variant.func_188046_a();

            if (this.models.get(resourcelocation) == null)
            {
                try
                {
                    this.models.put(resourcelocation, this.loadModel(resourcelocation));
                }
                catch (Exception exception)
                {
                    LOGGER.warn("Unable to load block model: \'{}\' for variant: \'{}\': {} ", new Object[] {resourcelocation, p_188638_1_, exception});
                }
            }
        }
    }

    private ModelBlock loadModel(ResourceLocation p_177594_1_) throws IOException
    {
        Reader reader = null;
        IResource iresource = null;
        ModelBlock lvt_5_1_;

        try
        {
            String s = p_177594_1_.getResourcePath();

            if (!"builtin/generated".equals(s))
            {
                if ("builtin/entity".equals(s))
                {
                    lvt_5_1_ = MODEL_ENTITY;
                    return lvt_5_1_;
                }

                if (s.startsWith("builtin/"))
                {
                    String s2 = s.substring("builtin/".length());
                    String s1 = (String)BUILT_IN_MODELS.get(s2);

                    if (s1 == null)
                    {
                        throw new FileNotFoundException(p_177594_1_.toString());
                    }

                    reader = new StringReader(s1);
                }
                else
                {
                    iresource = this.resourceManager.getResource(this.getModelLocation(p_177594_1_));
                    reader = new InputStreamReader(iresource.getInputStream(), Charsets.UTF_8);
                }

                lvt_5_1_ = ModelBlock.deserialize(reader);
                lvt_5_1_.name = p_177594_1_.toString();
                ModelBlock modelblock1 = lvt_5_1_;
                return modelblock1;
            }

            lvt_5_1_ = MODEL_GENERATED;
        }
        finally
        {
            IOUtils.closeQuietly(reader);
            IOUtils.closeQuietly((Closeable)iresource);
        }

        return lvt_5_1_;
    }

    private ResourceLocation getModelLocation(ResourceLocation p_177580_1_)
    {
        return new ResourceLocation(p_177580_1_.getResourceDomain(), "models/" + p_177580_1_.getResourcePath() + ".json");
    }

    private void loadItemModels()
    {
        this.registerVariantNames();

        for (Item item : Item.itemRegistry)
        {
            for (String s : this.getVariantNames(item))
            {
                ResourceLocation resourcelocation = this.getItemLocation(s);
                ResourceLocation resourcelocation1 = (ResourceLocation)Item.itemRegistry.getNameForObject(item);
                this.loadItemModel(s, resourcelocation, resourcelocation1);

                if (item.hasCustomProperties())
                {
                    ModelBlock modelblock = (ModelBlock)this.models.get(resourcelocation);

                    if (modelblock != null)
                    {
                        for (ResourceLocation resourcelocation2 : modelblock.getOverrideLocations())
                        {
                            this.loadItemModel(resourcelocation2.toString(), resourcelocation2, resourcelocation1);
                        }
                    }
                }
            }
        }
    }

    private void loadItemModel(String p_188634_1_, ResourceLocation p_188634_2_, ResourceLocation p_188634_3_)
    {
        this.itemLocations.put(p_188634_1_, p_188634_2_);

        if (this.models.get(p_188634_2_) == null)
        {
            try
            {
                ModelBlock modelblock = this.loadModel(p_188634_2_);
                this.models.put(p_188634_2_, modelblock);
            }
            catch (Exception exception)
            {
                LOGGER.warn((String)("Unable to load item model: \'" + p_188634_2_ + "\' for item: \'" + p_188634_3_ + "\'"), (Throwable)exception);
            }
        }
    }

    private void registerVariantNames()
    {
        this.variantNames.put(Item.getItemFromBlock(Blocks.stone), Lists.newArrayList(new String[] {"stone", "granite", "granite_smooth", "diorite", "diorite_smooth", "andesite", "andesite_smooth"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.dirt), Lists.newArrayList(new String[] {"dirt", "coarse_dirt", "podzol"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.planks), Lists.newArrayList(new String[] {"oak_planks", "spruce_planks", "birch_planks", "jungle_planks", "acacia_planks", "dark_oak_planks"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.sapling), Lists.newArrayList(new String[] {"oak_sapling", "spruce_sapling", "birch_sapling", "jungle_sapling", "acacia_sapling", "dark_oak_sapling"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.sand), Lists.newArrayList(new String[] {"sand", "red_sand"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.log), Lists.newArrayList(new String[] {"oak_log", "spruce_log", "birch_log", "jungle_log"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.leaves), Lists.newArrayList(new String[] {"oak_leaves", "spruce_leaves", "birch_leaves", "jungle_leaves"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.sponge), Lists.newArrayList(new String[] {"sponge", "sponge_wet"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.sandstone), Lists.newArrayList(new String[] {"sandstone", "chiseled_sandstone", "smooth_sandstone"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.red_sandstone), Lists.newArrayList(new String[] {"red_sandstone", "chiseled_red_sandstone", "smooth_red_sandstone"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.tallgrass), Lists.newArrayList(new String[] {"dead_bush", "tall_grass", "fern"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.deadbush), Lists.newArrayList(new String[] {"dead_bush"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.wool), Lists.newArrayList(new String[] {"black_wool", "red_wool", "green_wool", "brown_wool", "blue_wool", "purple_wool", "cyan_wool", "silver_wool", "gray_wool", "pink_wool", "lime_wool", "yellow_wool", "light_blue_wool", "magenta_wool", "orange_wool", "white_wool"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.yellow_flower), Lists.newArrayList(new String[] {"dandelion"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.red_flower), Lists.newArrayList(new String[] {"poppy", "blue_orchid", "allium", "houstonia", "red_tulip", "orange_tulip", "white_tulip", "pink_tulip", "oxeye_daisy"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stone_slab), Lists.newArrayList(new String[] {"stone_slab", "sandstone_slab", "cobblestone_slab", "brick_slab", "stone_brick_slab", "nether_brick_slab", "quartz_slab"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stone_slab2), Lists.newArrayList(new String[] {"red_sandstone_slab"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stained_glass), Lists.newArrayList(new String[] {"black_stained_glass", "red_stained_glass", "green_stained_glass", "brown_stained_glass", "blue_stained_glass", "purple_stained_glass", "cyan_stained_glass", "silver_stained_glass", "gray_stained_glass", "pink_stained_glass", "lime_stained_glass", "yellow_stained_glass", "light_blue_stained_glass", "magenta_stained_glass", "orange_stained_glass", "white_stained_glass"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.monster_egg), Lists.newArrayList(new String[] {"stone_monster_egg", "cobblestone_monster_egg", "stone_brick_monster_egg", "mossy_brick_monster_egg", "cracked_brick_monster_egg", "chiseled_brick_monster_egg"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stonebrick), Lists.newArrayList(new String[] {"stonebrick", "mossy_stonebrick", "cracked_stonebrick", "chiseled_stonebrick"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.wooden_slab), Lists.newArrayList(new String[] {"oak_slab", "spruce_slab", "birch_slab", "jungle_slab", "acacia_slab", "dark_oak_slab"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.cobblestone_wall), Lists.newArrayList(new String[] {"cobblestone_wall", "mossy_cobblestone_wall"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.anvil), Lists.newArrayList(new String[] {"anvil_intact", "anvil_slightly_damaged", "anvil_very_damaged"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.quartz_block), Lists.newArrayList(new String[] {"quartz_block", "chiseled_quartz_block", "quartz_column"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stained_hardened_clay), Lists.newArrayList(new String[] {"black_stained_hardened_clay", "red_stained_hardened_clay", "green_stained_hardened_clay", "brown_stained_hardened_clay", "blue_stained_hardened_clay", "purple_stained_hardened_clay", "cyan_stained_hardened_clay", "silver_stained_hardened_clay", "gray_stained_hardened_clay", "pink_stained_hardened_clay", "lime_stained_hardened_clay", "yellow_stained_hardened_clay", "light_blue_stained_hardened_clay", "magenta_stained_hardened_clay", "orange_stained_hardened_clay", "white_stained_hardened_clay"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.stained_glass_pane), Lists.newArrayList(new String[] {"black_stained_glass_pane", "red_stained_glass_pane", "green_stained_glass_pane", "brown_stained_glass_pane", "blue_stained_glass_pane", "purple_stained_glass_pane", "cyan_stained_glass_pane", "silver_stained_glass_pane", "gray_stained_glass_pane", "pink_stained_glass_pane", "lime_stained_glass_pane", "yellow_stained_glass_pane", "light_blue_stained_glass_pane", "magenta_stained_glass_pane", "orange_stained_glass_pane", "white_stained_glass_pane"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.leaves2), Lists.newArrayList(new String[] {"acacia_leaves", "dark_oak_leaves"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.log2), Lists.newArrayList(new String[] {"acacia_log", "dark_oak_log"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.prismarine), Lists.newArrayList(new String[] {"prismarine", "prismarine_bricks", "dark_prismarine"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.carpet), Lists.newArrayList(new String[] {"black_carpet", "red_carpet", "green_carpet", "brown_carpet", "blue_carpet", "purple_carpet", "cyan_carpet", "silver_carpet", "gray_carpet", "pink_carpet", "lime_carpet", "yellow_carpet", "light_blue_carpet", "magenta_carpet", "orange_carpet", "white_carpet"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.double_plant), Lists.newArrayList(new String[] {"sunflower", "syringa", "double_grass", "double_fern", "double_rose", "paeonia"}));
        this.variantNames.put(Items.coal, Lists.newArrayList(new String[] {"coal", "charcoal"}));
        this.variantNames.put(Items.fish, Lists.newArrayList(new String[] {"cod", "salmon", "clownfish", "pufferfish"}));
        this.variantNames.put(Items.cooked_fish, Lists.newArrayList(new String[] {"cooked_cod", "cooked_salmon"}));
        this.variantNames.put(Items.dye, Lists.newArrayList(new String[] {"dye_black", "dye_red", "dye_green", "dye_brown", "dye_blue", "dye_purple", "dye_cyan", "dye_silver", "dye_gray", "dye_pink", "dye_lime", "dye_yellow", "dye_light_blue", "dye_magenta", "dye_orange", "dye_white"}));
        this.variantNames.put(Items.potionitem, Lists.newArrayList(new String[] {"bottle_drinkable"}));
        this.variantNames.put(Items.skull, Lists.newArrayList(new String[] {"skull_skeleton", "skull_wither", "skull_zombie", "skull_char", "skull_creeper", "skull_dragon"}));
        this.variantNames.put(Items.splash_potion, Lists.newArrayList(new String[] {"bottle_splash"}));
        this.variantNames.put(Items.lingering_potion, Lists.newArrayList(new String[] {"bottle_lingering"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.oak_fence_gate), Lists.newArrayList(new String[] {"oak_fence_gate"}));
        this.variantNames.put(Item.getItemFromBlock(Blocks.oak_fence), Lists.newArrayList(new String[] {"oak_fence"}));
        this.variantNames.put(Items.oak_door, Lists.newArrayList(new String[] {"oak_door"}));
        this.variantNames.put(Items.boat, Lists.newArrayList(new String[] {"oak_boat"}));
    }

    private List<String> getVariantNames(Item p_177596_1_)
    {
        List<String> list = (List)this.variantNames.get(p_177596_1_);

        if (list == null)
        {
            list = Collections.<String>singletonList(((ResourceLocation)Item.itemRegistry.getNameForObject(p_177596_1_)).toString());
        }

        return list;
    }

    private ResourceLocation getItemLocation(String p_177583_1_)
    {
        ResourceLocation resourcelocation = new ResourceLocation(p_177583_1_);
        return new ResourceLocation(resourcelocation.getResourceDomain(), "item/" + resourcelocation.getResourcePath());
    }

    private void bakeBlockModels()
    {
        for (ModelResourceLocation modelresourcelocation : this.variants.keySet())
        {
            IBakedModel ibakedmodel = this.createRandomModelForVariantList((VariantList)this.variants.get(modelresourcelocation), modelresourcelocation.toString());

            if (ibakedmodel != null)
            {
                this.bakedRegistry.putObject(modelresourcelocation, ibakedmodel);
            }
        }

        for (Entry<ModelBlockDefinition, Collection<ModelResourceLocation>> entry : this.multipartVariantMap.entrySet())
        {
            ModelBlockDefinition modelblockdefinition = (ModelBlockDefinition)entry.getKey();
            Multipart multipart = modelblockdefinition.getMultipartData();
            String s = ((ResourceLocation)Block.blockRegistry.getNameForObject(multipart.func_188135_c().getBlock())).toString();
            MultipartBakedModel.Builder multipartbakedmodel$builder = new MultipartBakedModel.Builder();

            for (Selector selector : multipart.func_188136_a())
            {
                IBakedModel ibakedmodel1 = this.createRandomModelForVariantList(selector.func_188165_a(), "selector of " + s);

                if (ibakedmodel1 != null)
                {
                    multipartbakedmodel$builder.func_188648_a(selector.func_188166_a(multipart.func_188135_c()), ibakedmodel1);
                }
            }

            IBakedModel ibakedmodel2 = multipartbakedmodel$builder.func_188647_a();

            for (ModelResourceLocation modelresourcelocation1 : entry.getValue())
            {
                if (!modelblockdefinition.func_188000_b(modelresourcelocation1.getVariant()))
                {
                    this.bakedRegistry.putObject(modelresourcelocation1, ibakedmodel2);
                }
            }
        }
    }

    private IBakedModel createRandomModelForVariantList(VariantList p_188639_1_, String p_188639_2_)
    {
        if (p_188639_1_.func_188114_a().isEmpty())
        {
            return null;
        }
        else
        {
            WeightedBakedModel.Builder weightedbakedmodel$builder = new WeightedBakedModel.Builder();
            int i = 0;

            for (Variant variant : p_188639_1_.func_188114_a())
            {
                ModelBlock modelblock = (ModelBlock)this.models.get(variant.func_188046_a());

                if (modelblock != null && modelblock.isResolved())
                {
                    if (modelblock.getElements().isEmpty())
                    {
                        LOGGER.warn("Missing elements for: " + p_188639_2_);
                    }
                    else
                    {
                        IBakedModel ibakedmodel = this.bakeModel(modelblock, variant.func_188048_b(), variant.func_188049_c());

                        if (ibakedmodel != null)
                        {
                            ++i;
                            weightedbakedmodel$builder.add(ibakedmodel, variant.func_188047_d());
                        }
                    }
                }
                else
                {
                    LOGGER.warn("Missing model for: " + p_188639_2_);
                }
            }

            IBakedModel ibakedmodel1 = null;

            if (i == 0)
            {
                LOGGER.warn("No weighted models for: " + p_188639_2_);
            }
            else if (i == 1)
            {
                ibakedmodel1 = weightedbakedmodel$builder.first();
            }
            else
            {
                ibakedmodel1 = weightedbakedmodel$builder.build();
            }

            return ibakedmodel1;
        }
    }

    private void func_188635_i()
    {
        for (Entry<String, ResourceLocation> entry : this.itemLocations.entrySet())
        {
            ResourceLocation resourcelocation = (ResourceLocation)entry.getValue();
            ModelResourceLocation modelresourcelocation = new ModelResourceLocation((String)entry.getKey(), "inventory");
            ModelBlock modelblock = (ModelBlock)this.models.get(resourcelocation);

            if (modelblock != null && modelblock.isResolved())
            {
                if (modelblock.getElements().isEmpty())
                {
                    LOGGER.warn("Missing elements for: " + resourcelocation);
                }
                else if (this.isCustomRenderer(modelblock))
                {
                    this.bakedRegistry.putObject(modelresourcelocation, new BuiltInModel(modelblock.getAllTransforms(), modelblock.createOverrides()));
                }
                else
                {
                    IBakedModel ibakedmodel = this.bakeModel(modelblock, ModelRotation.X0_Y0, false);

                    if (ibakedmodel != null)
                    {
                        this.bakedRegistry.putObject(modelresourcelocation, ibakedmodel);
                    }
                }
            }
            else
            {
                LOGGER.warn("Missing model for: " + resourcelocation);
            }
        }
    }

    private Set<ResourceLocation> getVariantsTextureLocations()
    {
        Set<ResourceLocation> set = Sets.<ResourceLocation>newHashSet();
        List<ModelResourceLocation> list = Lists.newArrayList(this.variants.keySet());
        Collections.sort(list, new Comparator<ModelResourceLocation>()
        {
            public int compare(ModelResourceLocation p_compare_1_, ModelResourceLocation p_compare_2_)
            {
                return p_compare_1_.toString().compareTo(p_compare_2_.toString());
            }
        });

        for (ModelResourceLocation modelresourcelocation : list)
        {
            VariantList variantlist = (VariantList)this.variants.get(modelresourcelocation);

            for (Variant variant : variantlist.func_188114_a())
            {
                ModelBlock modelblock = (ModelBlock)this.models.get(variant.func_188046_a());

                if (modelblock == null)
                {
                    LOGGER.warn("Missing model for: " + modelresourcelocation);
                }
                else
                {
                    set.addAll(this.getTextureLocations(modelblock));
                }
            }
        }

        for (ModelBlockDefinition modelblockdefinition : this.multipartVariantMap.keySet())
        {
            for (VariantList variantlist1 : modelblockdefinition.getMultipartData().func_188137_b())
            {
                for (Variant variant1 : variantlist1.func_188114_a())
                {
                    ModelBlock modelblock1 = (ModelBlock)this.models.get(variant1.func_188046_a());

                    if (modelblock1 == null)
                    {
                        LOGGER.warn("Missing model for: " + Block.blockRegistry.getNameForObject(modelblockdefinition.getMultipartData().func_188135_c().getBlock()));
                    }
                    else
                    {
                        set.addAll(this.getTextureLocations(modelblock1));
                    }
                }
            }
        }

        set.addAll(LOCATIONS_BUILTIN_TEXTURES);
        return set;
    }

    private IBakedModel bakeModel(ModelBlock modelBlockIn, ModelRotation modelRotationIn, boolean uvLocked)
    {
        TextureAtlasSprite textureatlassprite = (TextureAtlasSprite)this.sprites.get(new ResourceLocation(modelBlockIn.resolveTextureName("particle")));
        SimpleBakedModel.Builder simplebakedmodel$builder = (new SimpleBakedModel.Builder(modelBlockIn, modelBlockIn.createOverrides())).setTexture(textureatlassprite);

        if (modelBlockIn.getElements().isEmpty())
        {
            return null;
        }
        else
        {
            for (BlockPart blockpart : modelBlockIn.getElements())
            {
                for (EnumFacing enumfacing : blockpart.mapFaces.keySet())
                {
                    BlockPartFace blockpartface = (BlockPartFace)blockpart.mapFaces.get(enumfacing);
                    TextureAtlasSprite textureatlassprite1 = (TextureAtlasSprite)this.sprites.get(new ResourceLocation(modelBlockIn.resolveTextureName(blockpartface.texture)));

                    if (blockpartface.cullFace == null)
                    {
                        simplebakedmodel$builder.addGeneralQuad(this.makeBakedQuad(blockpart, blockpartface, textureatlassprite1, enumfacing, modelRotationIn, uvLocked));
                    }
                    else
                    {
                        simplebakedmodel$builder.addFaceQuad(modelRotationIn.rotateFace(blockpartface.cullFace), this.makeBakedQuad(blockpart, blockpartface, textureatlassprite1, enumfacing, modelRotationIn, uvLocked));
                    }
                }
            }

            return simplebakedmodel$builder.makeBakedModel();
        }
    }

    private BakedQuad makeBakedQuad(BlockPart p_177589_1_, BlockPartFace p_177589_2_, TextureAtlasSprite p_177589_3_, EnumFacing p_177589_4_, ModelRotation p_177589_5_, boolean p_177589_6_)
    {
        return this.faceBakery.makeBakedQuad(p_177589_1_.positionFrom, p_177589_1_.positionTo, p_177589_2_, p_177589_3_, p_177589_4_, p_177589_5_, p_177589_1_.partRotation, p_177589_6_, p_177589_1_.shade);
    }

    private void loadModelsCheck()
    {
        this.loadModels();

        for (ModelBlock modelblock : this.models.values())
        {
            modelblock.getParentFromMap(this.models);
        }

        ModelBlock.checkModelHierarchy(this.models);
    }

    private void loadModels()
    {
        Deque<ResourceLocation> deque = Queues.<ResourceLocation>newArrayDeque();
        Set<ResourceLocation> set = Sets.<ResourceLocation>newHashSet();

        for (ResourceLocation resourcelocation : this.models.keySet())
        {
            set.add(resourcelocation);
            this.func_188633_a(deque, set, (ModelBlock)this.models.get(resourcelocation));
        }

        while (!((Deque)deque).isEmpty())
        {
            ResourceLocation resourcelocation1 = (ResourceLocation)deque.pop();

            try
            {
                if (this.models.get(resourcelocation1) != null)
                {
                    continue;
                }

                ModelBlock modelblock = this.loadModel(resourcelocation1);
                this.models.put(resourcelocation1, modelblock);
                this.func_188633_a(deque, set, modelblock);
            }
            catch (Exception exception)
            {
                LOGGER.warn((String)("In parent chain: " + JOINER.join(this.getParentPath(resourcelocation1)) + "; unable to load model: \'" + resourcelocation1 + "\'"), (Throwable)exception);
            }

            set.add(resourcelocation1);
        }
    }

    private void func_188633_a(Deque<ResourceLocation> p_188633_1_, Set<ResourceLocation> p_188633_2_, ModelBlock p_188633_3_)
    {
        ResourceLocation resourcelocation = p_188633_3_.getParentLocation();

        if (resourcelocation != null && !p_188633_2_.contains(resourcelocation))
        {
            p_188633_1_.add(resourcelocation);
        }
    }

    private List<ResourceLocation> getParentPath(ResourceLocation p_177573_1_)
    {
        List<ResourceLocation> list = Lists.newArrayList(new ResourceLocation[] {p_177573_1_});
        ResourceLocation resourcelocation = p_177573_1_;

        while ((resourcelocation = this.getParentLocation(resourcelocation)) != null)
        {
            list.add(0, resourcelocation);
        }

        return list;
    }

    private ResourceLocation getParentLocation(ResourceLocation p_177576_1_)
    {
        for (Entry<ResourceLocation, ModelBlock> entry : this.models.entrySet())
        {
            ModelBlock modelblock = (ModelBlock)entry.getValue();

            if (modelblock != null && p_177576_1_.equals(modelblock.getParentLocation()))
            {
                return (ResourceLocation)entry.getKey();
            }
        }

        return null;
    }

    private Set<ResourceLocation> getTextureLocations(ModelBlock p_177585_1_)
    {
        Set<ResourceLocation> set = Sets.<ResourceLocation>newHashSet();

        for (BlockPart blockpart : p_177585_1_.getElements())
        {
            for (BlockPartFace blockpartface : blockpart.mapFaces.values())
            {
                ResourceLocation resourcelocation = new ResourceLocation(p_177585_1_.resolveTextureName(blockpartface.texture));
                set.add(resourcelocation);
            }
        }

        set.add(new ResourceLocation(p_177585_1_.resolveTextureName("particle")));
        return set;
    }

    private void loadSprites()
    {
        final Set<ResourceLocation> set = this.getVariantsTextureLocations();
        set.addAll(this.getItemsTextureLocations());
        set.remove(TextureMap.LOCATION_MISSING_TEXTURE);
        IIconCreator iiconcreator = new IIconCreator()
        {
            public void registerSprites(TextureMap iconRegistry)
            {
                for (ResourceLocation resourcelocation : set)
                {
                    TextureAtlasSprite textureatlassprite = iconRegistry.registerSprite(resourcelocation);
                    ModelBakery.this.sprites.put(resourcelocation, textureatlassprite);
                }
            }
        };
        this.textureMap.loadSprites(this.resourceManager, iiconcreator);
        this.sprites.put(new ResourceLocation("missingno"), this.textureMap.getMissingSprite());
    }

    private Set<ResourceLocation> getItemsTextureLocations()
    {
        Set<ResourceLocation> set = Sets.<ResourceLocation>newHashSet();

        for (ResourceLocation resourcelocation : this.itemLocations.values())
        {
            ModelBlock modelblock = (ModelBlock)this.models.get(resourcelocation);

            if (modelblock != null)
            {
                set.add(new ResourceLocation(modelblock.resolveTextureName("particle")));

                if (this.hasItemModel(modelblock))
                {
                    for (String s : ItemModelGenerator.LAYERS)
                    {
                        set.add(new ResourceLocation(modelblock.resolveTextureName(s)));
                    }
                }
                else if (!this.isCustomRenderer(modelblock))
                {
                    for (BlockPart blockpart : modelblock.getElements())
                    {
                        for (BlockPartFace blockpartface : blockpart.mapFaces.values())
                        {
                            ResourceLocation resourcelocation1 = new ResourceLocation(modelblock.resolveTextureName(blockpartface.texture));
                            set.add(resourcelocation1);
                        }
                    }
                }
            }
        }

        return set;
    }

    private boolean hasItemModel(ModelBlock p_177581_1_)
    {
        return p_177581_1_ == null ? false : p_177581_1_.getRootModel() == MODEL_GENERATED;
    }

    private boolean isCustomRenderer(ModelBlock p_177587_1_)
    {
        if (p_177587_1_ == null)
        {
            return false;
        }
        else
        {
            ModelBlock modelblock = p_177587_1_.getRootModel();
            return modelblock == MODEL_ENTITY;
        }
    }

    private void bakeItemModels()
    {
        for (ResourceLocation resourcelocation : this.itemLocations.values())
        {
            ModelBlock modelblock = (ModelBlock)this.models.get(resourcelocation);

            if (this.hasItemModel(modelblock))
            {
                ModelBlock modelblock1 = this.makeItemModel(modelblock);

                if (modelblock1 != null)
                {
                    modelblock1.name = resourcelocation.toString();
                }

                this.models.put(resourcelocation, modelblock1);
            }
            else if (this.isCustomRenderer(modelblock))
            {
                this.models.put(resourcelocation, modelblock);
            }
        }

        for (TextureAtlasSprite textureatlassprite : this.sprites.values())
        {
            if (!textureatlassprite.hasAnimationMetadata())
            {
                textureatlassprite.clearFramesTextureData();
            }
        }
    }

    private ModelBlock makeItemModel(ModelBlock p_177582_1_)
    {
        return this.itemModelGenerator.makeItemModel(this.textureMap, p_177582_1_);
    }

    static
    {
        BUILT_IN_MODELS.put("missing", field_188641_d);
        MODEL_GENERATED.name = "generation marker";
        MODEL_ENTITY.name = "block entity marker";
    }
}
