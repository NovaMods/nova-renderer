package net.minecraft.block;

import com.google.common.collect.Sets;
import java.util.List;
import java.util.Random;
import java.util.Set;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.enchantment.EnchantmentHelper;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.item.EntityXPOrb;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Enchantments;
import net.minecraft.item.Item;
import net.minecraft.item.ItemBlock;
import net.minecraft.item.ItemStack;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.Mirror;
import net.minecraft.util.ObjectIntIdentityMap;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.registry.RegistryNamespacedDefaultedByKey;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.Explosion;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class Block
{
    /** ResourceLocation for the Air block */
    private static final ResourceLocation AIR_ID = new ResourceLocation("air");
    public static final RegistryNamespacedDefaultedByKey<ResourceLocation, Block> blockRegistry = new RegistryNamespacedDefaultedByKey(AIR_ID);
    public static final ObjectIntIdentityMap<IBlockState> BLOCK_STATE_IDS = new ObjectIntIdentityMap();
    public static final AxisAlignedBB FULL_BLOCK_AABB = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D);
    public static final AxisAlignedBB NULL_AABB = null;
    private CreativeTabs displayOnCreativeTab;
    protected boolean fullBlock;

    /** How much light is subtracted for going through this block */
    protected int lightOpacity;
    protected boolean translucent;

    /** Amount of light emitted */
    protected int lightValue;

    /**
     * Flag if block should use the brightest neighbor light value as its own
     */
    protected boolean useNeighborBrightness;

    /** Indicates how many hits it takes to break a block. */
    protected float blockHardness;

    /** Indicates how much this block can resist explosions */
    protected float blockResistance;
    protected boolean enableStats;

    /**
     * Flags whether or not this block is of a type that needs random ticking. Ref-counted by ExtendedBlockStorage in
     * order to broadly cull a chunk from the random chunk update list for efficiency's sake.
     */
    protected boolean needsRandomTick;

    /** true if the Block contains a Tile Entity */
    protected boolean isBlockContainer;

    /** Sound of stepping on the block */
    protected SoundType stepSound;
    public float blockParticleGravity;
    protected final Material blockMaterial;

    /** The Block's MapColor */
    protected final MapColor blockMapColor;

    /**
     * Determines how much velocity is maintained while moving on top of this block
     */
    public float slipperiness;
    protected final BlockStateContainer blockState;
    private IBlockState defaultBlockState;
    private String unlocalizedName;

    public static int getIdFromBlock(Block blockIn)
    {
        return blockRegistry.getIDForObject(blockIn);
    }

    /**
     * Get a unique ID for the given BlockState, containing both BlockID and metadata
     */
    public static int getStateId(IBlockState state)
    {
        Block block = state.getBlock();
        return getIdFromBlock(block) + (block.getMetaFromState(state) << 12);
    }

    public static Block getBlockById(int id)
    {
        return (Block)blockRegistry.getObjectById(id);
    }

    /**
     * Get a BlockState by it's ID (see getStateId)
     */
    public static IBlockState getStateById(int id)
    {
        int i = id & 4095;
        int j = id >> 12 & 15;
        return getBlockById(i).getStateFromMeta(j);
    }

    public static Block getBlockFromItem(Item itemIn)
    {
        return itemIn instanceof ItemBlock ? ((ItemBlock)itemIn).getBlock() : null;
    }

    public static Block getBlockFromName(String name)
    {
        ResourceLocation resourcelocation = new ResourceLocation(name);

        if (blockRegistry.containsKey(resourcelocation))
        {
            return (Block)blockRegistry.getObject(resourcelocation);
        }
        else
        {
            try
            {
                return (Block)blockRegistry.getObjectById(Integer.parseInt(name));
            }
            catch (NumberFormatException var3)
            {
                return null;
            }
        }
    }

    /**
     * Checks if an IBlockState represents a block that is opaque and a full cube.
     *  
     * @param state The block state to check.
     */
    public boolean isFullyOpaque(IBlockState state)
    {
        return state.getMaterial().isOpaque() && state.isFullCube();
    }

    public boolean isFullBlock(IBlockState state)
    {
        return this.fullBlock;
    }

    public int getLightOpacity(IBlockState state)
    {
        return this.lightOpacity;
    }

    /**
     * Used in the renderer to apply ambient occlusion
     */
    public boolean isTranslucent(IBlockState state)
    {
        return this.translucent;
    }

    public int getLightValue(IBlockState state)
    {
        return this.lightValue;
    }

    /**
     * Should block use the brightest neighbor light value as its own
     */
    public boolean getUseNeighborBrightness(IBlockState state)
    {
        return this.useNeighborBrightness;
    }

    /**
     * Get a material of block
     */
    public Material getMaterial(IBlockState state)
    {
        return this.blockMaterial;
    }

    /**
     * Get the MapColor for this Block and the given BlockState
     */
    public MapColor getMapColor(IBlockState state)
    {
        return this.blockMapColor;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState();
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        if (state != null && !state.getPropertyNames().isEmpty())
        {
            throw new IllegalArgumentException("Don\'t know how to convert " + state + " back into data...");
        }
        else
        {
            return 0;
        }
    }

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        return state;
    }

    /**
     * Returns the blockstate with the given rotation from the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withRotation(IBlockState state, Rotation rot)
    {
        return state;
    }

    /**
     * Returns the blockstate with the given mirror of the passed blockstate. If inapplicable, returns the passed
     * blockstate.
     */
    public IBlockState withMirror(IBlockState state, Mirror mirrorIn)
    {
        return state;
    }

    public Block(Material blockMaterialIn, MapColor blockMapColorIn)
    {
        this.enableStats = true;
        this.stepSound = SoundType.STONE;
        this.blockParticleGravity = 1.0F;
        this.slipperiness = 0.6F;
        this.blockMaterial = blockMaterialIn;
        this.blockMapColor = blockMapColorIn;
        this.blockState = this.createBlockState();
        this.setDefaultState(this.blockState.getBaseState());
        this.fullBlock = this.getDefaultState().isOpaqueCube();
        this.lightOpacity = this.fullBlock ? 255 : 0;
        this.translucent = !blockMaterialIn.blocksLight();
    }

    protected Block(Material materialIn)
    {
        this(materialIn, materialIn.getMaterialMapColor());
    }

    /**
     * Sets the footstep sound for the block. Returns the object for convenience in constructing.
     */
    protected Block setStepSound(SoundType sound)
    {
        this.stepSound = sound;
        return this;
    }

    /**
     * Sets how much light is blocked going through this block. Returns the object for convenience in constructing.
     */
    protected Block setLightOpacity(int opacity)
    {
        this.lightOpacity = opacity;
        return this;
    }

    /**
     * Sets the light value that the block emits. Returns resulting block instance for constructing convenience. Args:
     * level
     */
    protected Block setLightLevel(float value)
    {
        this.lightValue = (int)(15.0F * value);
        return this;
    }

    /**
     * Sets the the blocks resistance to explosions. Returns the object for convenience in constructing.
     */
    protected Block setResistance(float resistance)
    {
        this.blockResistance = resistance * 3.0F;
        return this;
    }

    /**
     * Indicate if a material is a normal solid opaque cube
     */
    public boolean isBlockNormalCube(IBlockState state)
    {
        return state.getMaterial().blocksMovement() && state.isFullCube();
    }

    /**
     * Used for nearly all game logic (non-rendering) purposes. Use Forge-provided isNormalCube(IBlockAccess, BlockPos)
     * instead.
     */
    public boolean isNormalCube(IBlockState state)
    {
        return state.getMaterial().isOpaque() && state.isFullCube() && !state.canProvidePower();
    }

    public boolean isVisuallyOpaque()
    {
        return this.blockMaterial.blocksMovement() && this.getDefaultState().isFullCube();
    }

    public boolean isFullCube(IBlockState state)
    {
        return true;
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return !this.blockMaterial.blocksMovement();
    }

    /**
     * The type of render function called. 3 for standard block models, 2 for TESR's, 1 for liquids, -1 is no render
     */
    public EnumBlockRenderType getRenderType(IBlockState state)
    {
        return EnumBlockRenderType.MODEL;
    }

    /**
     * Whether this Block can be replaced directly by other blocks (true for e.g. tall grass)
     */
    public boolean isReplaceable(IBlockAccess worldIn, BlockPos pos)
    {
        return false;
    }

    /**
     * Sets how many hits it takes to break a block.
     */
    protected Block setHardness(float hardness)
    {
        this.blockHardness = hardness;

        if (this.blockResistance < hardness * 5.0F)
        {
            this.blockResistance = hardness * 5.0F;
        }

        return this;
    }

    protected Block setBlockUnbreakable()
    {
        this.setHardness(-1.0F);
        return this;
    }

    public float getBlockHardness(IBlockState worldIn, World pos, BlockPos state)
    {
        return this.blockHardness;
    }

    /**
     * Sets whether this block type will receive random update ticks
     */
    protected Block setTickRandomly(boolean shouldTick)
    {
        this.needsRandomTick = shouldTick;
        return this;
    }

    /**
     * Returns whether or not this block is of a type that needs random ticking. Called for ref-counting purposes by
     * ExtendedBlockStorage in order to broadly cull a chunk from the random chunk update list for efficiency's sake.
     */
    public boolean getTickRandomly()
    {
        return this.needsRandomTick;
    }

    public boolean hasTileEntity()
    {
        return this.isBlockContainer;
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return FULL_BLOCK_AABB;
    }

    public int getPackedLightmapCoords(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        int i = source.getCombinedLight(pos, state.getlightValue());

        if (i == 0 && state.getBlock() instanceof BlockSlab)
        {
            pos = pos.down();
            state = source.getBlockState(pos);
            return source.getCombinedLight(pos, state.getlightValue());
        }
        else
        {
            return i;
        }
    }

    public boolean shouldSideBeRendered(IBlockState worldIn, IBlockAccess pos, BlockPos side, EnumFacing facing)
    {
        AxisAlignedBB axisalignedbb = worldIn.func_185900_c(pos, side);

        switch (facing)
        {
            case DOWN:
                if (axisalignedbb.minY > 0.0D)
                {
                    return true;
                }

                break;

            case UP:
                if (axisalignedbb.maxY < 1.0D)
                {
                    return true;
                }

                break;

            case NORTH:
                if (axisalignedbb.minZ > 0.0D)
                {
                    return true;
                }

                break;

            case SOUTH:
                if (axisalignedbb.maxZ < 1.0D)
                {
                    return true;
                }

                break;

            case WEST:
                if (axisalignedbb.minX > 0.0D)
                {
                    return true;
                }

                break;

            case EAST:
                if (axisalignedbb.maxX < 1.0D)
                {
                    return true;
                }
        }

        return !pos.getBlockState(side.offset(facing)).isOpaqueCube();
    }

    /**
     * Whether this Block is solid on the given Side
     */
    public boolean isBlockSolid(IBlockAccess worldIn, BlockPos pos, EnumFacing side)
    {
        return worldIn.getBlockState(pos).getMaterial().isSolid();
    }

    public AxisAlignedBB getCollisionBoundingBox(IBlockState worldIn, World pos, BlockPos state)
    {
        return worldIn.func_185900_c(pos, state).offset(state);
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        func_185492_a(pos, p_185477_4_, p_185477_5_, state.getSelectedBoundingBox(worldIn, pos));
    }

    protected static void func_185492_a(BlockPos pos, AxisAlignedBB p_185492_1_, List<AxisAlignedBB> p_185492_2_, AxisAlignedBB p_185492_3_)
    {
        if (p_185492_3_ != NULL_AABB)
        {
            AxisAlignedBB axisalignedbb = p_185492_3_.offset(pos);

            if (p_185492_1_.intersectsWith(axisalignedbb))
            {
                p_185492_2_.add(axisalignedbb);
            }
        }
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return worldIn.func_185900_c(pos, p_180646_3_);
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return true;
    }

    public boolean canCollideCheck(IBlockState state, boolean hitIfLiquid)
    {
        return this.isCollidable();
    }

    /**
     * Returns if this block is collidable (only used by Fire). Args: x, y, z
     */
    public boolean isCollidable()
    {
        return true;
    }

    /**
     * Called randomly when setTickRandomly is set to true (used by e.g. crops to grow, etc.)
     */
    public void randomTick(World worldIn, BlockPos pos, IBlockState state, Random random)
    {
        this.updateTick(worldIn, pos, state, random);
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
    }

    /**
     * Called when a player destroys this Block
     */
    public void onBlockDestroyedByPlayer(World worldIn, BlockPos pos, IBlockState state)
    {
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 10;
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
    }

    public void breakBlock(World worldIn, BlockPos pos, IBlockState state)
    {
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 1;
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Item.getItemFromBlock(this);
    }

    /**
     * Get the hardness of this Block relative to the ability of the given player
     */
    public float getPlayerRelativeBlockHardness(IBlockState playerIn, EntityPlayer worldIn, World pos, BlockPos p_180647_4_)
    {
        float f = playerIn.getBlockHardness(pos, p_180647_4_);
        return f < 0.0F ? 0.0F : (!worldIn.func_184823_b(playerIn) ? worldIn.func_184813_a(playerIn) / f / 100.0F : worldIn.func_184813_a(playerIn) / f / 30.0F);
    }

    /**
     * Spawn this Block's drops into the World as EntityItems
     */
    public final void dropBlockAsItem(World worldIn, BlockPos pos, IBlockState state, int forture)
    {
        this.dropBlockAsItemWithChance(worldIn, pos, state, 1.0F, forture);
    }

    /**
     * Spawns this Block's drops into the World as EntityItems.
     */
    public void dropBlockAsItemWithChance(World worldIn, BlockPos pos, IBlockState state, float chance, int fortune)
    {
        if (!worldIn.isRemote)
        {
            int i = this.quantityDroppedWithBonus(fortune, worldIn.rand);

            for (int j = 0; j < i; ++j)
            {
                if (worldIn.rand.nextFloat() <= chance)
                {
                    Item item = this.getItemDropped(state, worldIn.rand, fortune);

                    if (item != null)
                    {
                        spawnAsEntity(worldIn, pos, new ItemStack(item, 1, this.damageDropped(state)));
                    }
                }
            }
        }
    }

    /**
     * Spawns the given ItemStack as an EntityItem into the World at the given position
     */
    public static void spawnAsEntity(World worldIn, BlockPos pos, ItemStack stack)
    {
        if (!worldIn.isRemote && worldIn.getGameRules().getBoolean("doTileDrops"))
        {
            float f = 0.5F;
            double d0 = (double)(worldIn.rand.nextFloat() * f) + (double)(1.0F - f) * 0.5D;
            double d1 = (double)(worldIn.rand.nextFloat() * f) + (double)(1.0F - f) * 0.5D;
            double d2 = (double)(worldIn.rand.nextFloat() * f) + (double)(1.0F - f) * 0.5D;
            EntityItem entityitem = new EntityItem(worldIn, (double)pos.getX() + d0, (double)pos.getY() + d1, (double)pos.getZ() + d2, stack);
            entityitem.setDefaultPickupDelay();
            worldIn.spawnEntityInWorld(entityitem);
        }
    }

    /**
     * Spawns the given amount of experience into the World as XP orb entities
     */
    protected void dropXpOnBlockBreak(World worldIn, BlockPos pos, int amount)
    {
        if (!worldIn.isRemote && worldIn.getGameRules().getBoolean("doTileDrops"))
        {
            while (amount > 0)
            {
                int i = EntityXPOrb.getXPSplit(amount);
                amount -= i;
                worldIn.spawnEntityInWorld(new EntityXPOrb(worldIn, (double)pos.getX() + 0.5D, (double)pos.getY() + 0.5D, (double)pos.getZ() + 0.5D, i));
            }
        }
    }

    /**
     * Gets the metadata of the item this Block can drop. This method is called when the block gets destroyed. It
     * returns the metadata of the dropped item based on the old metadata of the block.
     */
    public int damageDropped(IBlockState state)
    {
        return 0;
    }

    /**
     * Returns how much this block can resist explosions from the passed in entity.
     */
    public float getExplosionResistance(Entity exploder)
    {
        return this.blockResistance / 5.0F;
    }

    /**
     * Ray traces through the blocks collision from start vector to end vector returning a ray trace hit.
     */
    public RayTraceResult collisionRayTrace(IBlockState worldIn, World pos, BlockPos start, Vec3d end, Vec3d p_180636_5_)
    {
        return this.func_185503_a(start, end, p_180636_5_, worldIn.func_185900_c(pos, start));
    }

    protected RayTraceResult func_185503_a(BlockPos p_185503_1_, Vec3d p_185503_2_, Vec3d p_185503_3_, AxisAlignedBB p_185503_4_)
    {
        Vec3d vec3d = p_185503_2_.subtract((double)p_185503_1_.getX(), (double)p_185503_1_.getY(), (double)p_185503_1_.getZ());
        Vec3d vec3d1 = p_185503_3_.subtract((double)p_185503_1_.getX(), (double)p_185503_1_.getY(), (double)p_185503_1_.getZ());
        RayTraceResult raytraceresult = p_185503_4_.calculateIntercept(vec3d, vec3d1);
        return raytraceresult == null ? null : new RayTraceResult(raytraceresult.hitVec.addVector((double)p_185503_1_.getX(), (double)p_185503_1_.getY(), (double)p_185503_1_.getZ()), raytraceresult.sideHit, p_185503_1_);
    }

    /**
     * Called when this Block is destroyed by an Explosion
     */
    public void onBlockDestroyedByExplosion(World worldIn, BlockPos pos, Explosion explosionIn)
    {
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.SOLID;
    }

    public boolean canReplace(World worldIn, BlockPos pos, EnumFacing side, ItemStack stack)
    {
        return this.canPlaceBlockOnSide(worldIn, pos, side);
    }

    /**
     * Check whether this Block can be placed on the given side
     */
    public boolean canPlaceBlockOnSide(World worldIn, BlockPos pos, EnumFacing side)
    {
        return this.canPlaceBlockAt(worldIn, pos);
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return worldIn.getBlockState(pos).getBlock().blockMaterial.isReplaceable();
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        return false;
    }

    /**
     * Triggered whenever an entity collides with this block (enters into the block)
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, Entity entityIn)
    {
    }

    /**
     * Called by ItemBlocks just before a block is actually set in the world, to allow for adjustments to the
     * IBlockstate
     */
    public IBlockState onBlockPlaced(World worldIn, BlockPos pos, EnumFacing facing, float hitX, float hitY, float hitZ, int meta, EntityLivingBase placer)
    {
        return this.getStateFromMeta(meta);
    }

    public void onBlockClicked(World worldIn, BlockPos pos, EntityPlayer playerIn)
    {
    }

    public Vec3d modifyAcceleration(World worldIn, BlockPos pos, Entity entityIn, Vec3d motion)
    {
        return motion;
    }

    public int getWeakPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return 0;
    }

    /**
     * Can this block provide power. Only wire currently seems to have this change based on its state.
     */
    public boolean canProvidePower(IBlockState state)
    {
        return false;
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
    }

    public int getStrongPower(IBlockState worldIn, IBlockAccess pos, BlockPos state, EnumFacing side)
    {
        return 0;
    }

    public void harvestBlock(World worldIn, EntityPlayer player, BlockPos pos, IBlockState state, TileEntity te, ItemStack p_180657_6_)
    {
        player.triggerAchievement(StatList.func_188055_a(this));
        player.addExhaustion(0.025F);

        if (this.canSilkHarvest() && EnchantmentHelper.getEnchantmentLevel(Enchantments.silkTouch, p_180657_6_) > 0)
        {
            ItemStack itemstack = this.createStackedBlock(state);

            if (itemstack != null)
            {
                spawnAsEntity(worldIn, pos, itemstack);
            }
        }
        else
        {
            int i = EnchantmentHelper.getEnchantmentLevel(Enchantments.fortune, p_180657_6_);
            this.dropBlockAsItem(worldIn, pos, state, i);
        }
    }

    protected boolean canSilkHarvest()
    {
        return this.getDefaultState().isFullCube() && !this.isBlockContainer;
    }

    protected ItemStack createStackedBlock(IBlockState state)
    {
        Item item = Item.getItemFromBlock(this);

        if (item == null)
        {
            return null;
        }
        else
        {
            int i = 0;

            if (item.getHasSubtypes())
            {
                i = this.getMetaFromState(state);
            }

            return new ItemStack(item, 1, i);
        }
    }

    /**
     * Get the quantity dropped based on the given fortune level
     */
    public int quantityDroppedWithBonus(int fortune, Random random)
    {
        return this.quantityDropped(random);
    }

    /**
     * Called by ItemBlocks after a block is set in the world, to allow post-place logic
     */
    public void onBlockPlacedBy(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase placer, ItemStack stack)
    {
    }

    /**
     * Return true if an entity can be spawned inside the block (used to get the player's bed spawn location)
     */
    public boolean canSpawnInBlock()
    {
        return !this.blockMaterial.isSolid() && !this.blockMaterial.isLiquid();
    }

    public Block setUnlocalizedName(String name)
    {
        this.unlocalizedName = name;
        return this;
    }

    /**
     * Gets the localized name of this block. Used for the statistics page.
     */
    public String getLocalizedName()
    {
        return I18n.translateToLocal(this.getUnlocalizedName() + ".name");
    }

    /**
     * Returns the unlocalized name of the block with "tile." appended to the front.
     */
    public String getUnlocalizedName()
    {
        return "tile." + this.unlocalizedName;
    }

    /**
     * Called on both Client and Server when World#addBlockEvent is called
     */
    public boolean onBlockEventReceived(World worldIn, BlockPos pos, IBlockState state, int eventID, int eventParam)
    {
        return false;
    }

    /**
     * Return the state of blocks statistics flags - if the block is counted for mined and placed.
     */
    public boolean getEnableStats()
    {
        return this.enableStats;
    }

    protected Block disableStats()
    {
        this.enableStats = false;
        return this;
    }

    public EnumPushReaction getMobilityFlag(IBlockState state)
    {
        return this.blockMaterial.func_186274_m();
    }

    public float func_185485_f(IBlockState state)
    {
        return state.isBlockNormalCube() ? 0.2F : 1.0F;
    }

    /**
     * Block's chance to react to a living entity falling on it.
     */
    public void onFallenUpon(World worldIn, BlockPos pos, Entity entityIn, float fallDistance)
    {
        entityIn.fall(fallDistance, 1.0F);
    }

    /**
     * Called when an Entity lands on this Block. This method *must* update motionY because the entity will not do that
     * on its own
     */
    public void onLanded(World worldIn, Entity entityIn)
    {
        entityIn.motionY = 0.0D;
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(Item.getItemFromBlock(this), 1, this.damageDropped(state));
    }

    /**
     * returns a list of blocks with the same ID, but different meta (eg: wood returns 4 blocks)
     */
    public void getSubBlocks(Item itemIn, CreativeTabs tab, List<ItemStack> list)
    {
        list.add(new ItemStack(itemIn));
    }

    /**
     * Returns the CreativeTab to display the given block on.
     */
    public CreativeTabs getCreativeTabToDisplayOn()
    {
        return this.displayOnCreativeTab;
    }

    public Block setCreativeTab(CreativeTabs tab)
    {
        this.displayOnCreativeTab = tab;
        return this;
    }

    public void onBlockHarvested(World worldIn, BlockPos pos, IBlockState state, EntityPlayer player)
    {
    }

    /**
     * Called similar to random ticks, but only when it is raining.
     */
    public void fillWithRain(World worldIn, BlockPos pos)
    {
    }

    public boolean requiresUpdates()
    {
        return true;
    }

    /**
     * Return whether this block can drop from an explosion.
     */
    public boolean canDropFromExplosion(Explosion explosionIn)
    {
        return true;
    }

    public boolean isAssociatedBlock(Block other)
    {
        return this == other;
    }

    public static boolean isEqualTo(Block blockIn, Block other)
    {
        return blockIn != null && other != null ? (blockIn == other ? true : blockIn.isAssociatedBlock(other)) : false;
    }

    public boolean hasComparatorInputOverride(IBlockState state)
    {
        return false;
    }

    public int getComparatorInputOverride(IBlockState worldIn, World pos, BlockPos p_180641_3_)
    {
        return 0;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[0]);
    }

    public BlockStateContainer getBlockState()
    {
        return this.blockState;
    }

    protected final void setDefaultState(IBlockState state)
    {
        this.defaultBlockState = state;
    }

    public final IBlockState getDefaultState()
    {
        return this.defaultBlockState;
    }

    /**
     * Get the OffsetType for this Block. Determines if the model is rendered slightly offset.
     */
    public Block.EnumOffsetType getOffsetType()
    {
        return Block.EnumOffsetType.NONE;
    }

    public SoundType getStepSound()
    {
        return this.stepSound;
    }

    public String toString()
    {
        return "Block{" + blockRegistry.getNameForObject(this) + "}";
    }

    public static void registerBlocks()
    {
        registerBlock(0, AIR_ID, (new BlockAir()).setUnlocalizedName("air"));
        registerBlock(1, "stone", (new BlockStone()).setHardness(1.5F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stone"));
        registerBlock(2, "grass", (new BlockGrass()).setHardness(0.6F).setStepSound(SoundType.PLANT).setUnlocalizedName("grass"));
        registerBlock(3, "dirt", (new BlockDirt()).setHardness(0.5F).setStepSound(SoundType.GROUND).setUnlocalizedName("dirt"));
        Block block = (new Block(Material.rock)).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stonebrick").setCreativeTab(CreativeTabs.tabBlock);
        registerBlock(4, "cobblestone", block);
        Block block1 = (new BlockPlanks()).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("wood");
        registerBlock(5, "planks", block1);
        registerBlock(6, "sapling", (new BlockSapling()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("sapling"));
        registerBlock(7, "bedrock", (new BlockEmptyDrops(Material.rock)).setBlockUnbreakable().setResistance(6000000.0F).setStepSound(SoundType.STONE).setUnlocalizedName("bedrock").disableStats().setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(8, "flowing_water", (new BlockDynamicLiquid(Material.water)).setHardness(100.0F).setLightOpacity(3).setUnlocalizedName("water").disableStats());
        registerBlock(9, "water", (new BlockStaticLiquid(Material.water)).setHardness(100.0F).setLightOpacity(3).setUnlocalizedName("water").disableStats());
        registerBlock(10, "flowing_lava", (new BlockDynamicLiquid(Material.lava)).setHardness(100.0F).setLightLevel(1.0F).setUnlocalizedName("lava").disableStats());
        registerBlock(11, "lava", (new BlockStaticLiquid(Material.lava)).setHardness(100.0F).setLightLevel(1.0F).setUnlocalizedName("lava").disableStats());
        registerBlock(12, "sand", (new BlockSand()).setHardness(0.5F).setStepSound(SoundType.SAND).setUnlocalizedName("sand"));
        registerBlock(13, "gravel", (new BlockGravel()).setHardness(0.6F).setStepSound(SoundType.GROUND).setUnlocalizedName("gravel"));
        registerBlock(14, "gold_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreGold"));
        registerBlock(15, "iron_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreIron"));
        registerBlock(16, "coal_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreCoal"));
        registerBlock(17, "log", (new BlockOldLog()).setUnlocalizedName("log"));
        registerBlock(18, "leaves", (new BlockOldLeaf()).setUnlocalizedName("leaves"));
        registerBlock(19, "sponge", (new BlockSponge()).setHardness(0.6F).setStepSound(SoundType.PLANT).setUnlocalizedName("sponge"));
        registerBlock(20, "glass", (new BlockGlass(Material.glass, false)).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("glass"));
        registerBlock(21, "lapis_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreLapis"));
        registerBlock(22, "lapis_block", (new Block(Material.iron, MapColor.lapisColor)).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("blockLapis").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(23, "dispenser", (new BlockDispenser()).setHardness(3.5F).setStepSound(SoundType.STONE).setUnlocalizedName("dispenser"));
        Block block2 = (new BlockSandStone()).setStepSound(SoundType.STONE).setHardness(0.8F).setUnlocalizedName("sandStone");
        registerBlock(24, "sandstone", block2);
        registerBlock(25, "noteblock", (new BlockNote()).setStepSound(SoundType.WOOD).setHardness(0.8F).setUnlocalizedName("musicBlock"));
        registerBlock(26, "bed", (new BlockBed()).setStepSound(SoundType.WOOD).setHardness(0.2F).setUnlocalizedName("bed").disableStats());
        registerBlock(27, "golden_rail", (new BlockRailPowered()).setHardness(0.7F).setStepSound(SoundType.METAL).setUnlocalizedName("goldenRail"));
        registerBlock(28, "detector_rail", (new BlockRailDetector()).setHardness(0.7F).setStepSound(SoundType.METAL).setUnlocalizedName("detectorRail"));
        registerBlock(29, "sticky_piston", (new BlockPistonBase(true)).setUnlocalizedName("pistonStickyBase"));
        registerBlock(30, "web", (new BlockWeb()).setLightOpacity(1).setHardness(4.0F).setUnlocalizedName("web"));
        registerBlock(31, "tallgrass", (new BlockTallGrass()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("tallgrass"));
        registerBlock(32, "deadbush", (new BlockDeadBush()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("deadbush"));
        registerBlock(33, "piston", (new BlockPistonBase(false)).setUnlocalizedName("pistonBase"));
        registerBlock(34, "piston_head", (new BlockPistonExtension()).setUnlocalizedName("pistonBase"));
        registerBlock(35, "wool", (new BlockColored(Material.cloth)).setHardness(0.8F).setStepSound(SoundType.CLOTH).setUnlocalizedName("cloth"));
        registerBlock(36, "piston_extension", new BlockPistonMoving());
        registerBlock(37, "yellow_flower", (new BlockYellowFlower()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("flower1"));
        registerBlock(38, "red_flower", (new BlockRedFlower()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("flower2"));
        Block block3 = (new BlockMushroom()).setHardness(0.0F).setStepSound(SoundType.PLANT).setLightLevel(0.125F).setUnlocalizedName("mushroom");
        registerBlock(39, "brown_mushroom", block3);
        Block block4 = (new BlockMushroom()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("mushroom");
        registerBlock(40, "red_mushroom", block4);
        registerBlock(41, "gold_block", (new Block(Material.iron, MapColor.goldColor)).setHardness(3.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("blockGold").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(42, "iron_block", (new Block(Material.iron, MapColor.ironColor)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("blockIron").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(43, "double_stone_slab", (new BlockDoubleStoneSlab()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stoneSlab"));
        registerBlock(44, "stone_slab", (new BlockHalfStoneSlab()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stoneSlab"));
        Block block5 = (new Block(Material.rock, MapColor.redColor)).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("brick").setCreativeTab(CreativeTabs.tabBlock);
        registerBlock(45, "brick_block", block5);
        registerBlock(46, "tnt", (new BlockTNT()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("tnt"));
        registerBlock(47, "bookshelf", (new BlockBookshelf()).setHardness(1.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("bookshelf"));
        registerBlock(48, "mossy_cobblestone", (new Block(Material.rock)).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stoneMoss").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(49, "obsidian", (new BlockObsidian()).setHardness(50.0F).setResistance(2000.0F).setStepSound(SoundType.STONE).setUnlocalizedName("obsidian"));
        registerBlock(50, "torch", (new BlockTorch()).setHardness(0.0F).setLightLevel(0.9375F).setStepSound(SoundType.WOOD).setUnlocalizedName("torch"));
        registerBlock(51, "fire", (new BlockFire()).setHardness(0.0F).setLightLevel(1.0F).setStepSound(SoundType.CLOTH).setUnlocalizedName("fire").disableStats());
        registerBlock(52, "mob_spawner", (new BlockMobSpawner()).setHardness(5.0F).setStepSound(SoundType.METAL).setUnlocalizedName("mobSpawner").disableStats());
        registerBlock(53, "oak_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.OAK))).setUnlocalizedName("stairsWood"));
        registerBlock(54, "chest", (new BlockChest(BlockChest.Type.BASIC)).setHardness(2.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("chest"));
        registerBlock(55, "redstone_wire", (new BlockRedstoneWire()).setHardness(0.0F).setStepSound(SoundType.STONE).setUnlocalizedName("redstoneDust").disableStats());
        registerBlock(56, "diamond_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreDiamond"));
        registerBlock(57, "diamond_block", (new Block(Material.iron, MapColor.diamondColor)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("blockDiamond").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(58, "crafting_table", (new BlockWorkbench()).setHardness(2.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("workbench"));
        registerBlock(59, "wheat", (new BlockCrops()).setUnlocalizedName("crops"));
        Block block6 = (new BlockFarmland()).setHardness(0.6F).setStepSound(SoundType.GROUND).setUnlocalizedName("farmland");
        registerBlock(60, "farmland", block6);
        registerBlock(61, "furnace", (new BlockFurnace(false)).setHardness(3.5F).setStepSound(SoundType.STONE).setUnlocalizedName("furnace").setCreativeTab(CreativeTabs.tabDecorations));
        registerBlock(62, "lit_furnace", (new BlockFurnace(true)).setHardness(3.5F).setStepSound(SoundType.STONE).setLightLevel(0.875F).setUnlocalizedName("furnace"));
        registerBlock(63, "standing_sign", (new BlockStandingSign()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("sign").disableStats());
        registerBlock(64, "wooden_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorOak").disableStats());
        registerBlock(65, "ladder", (new BlockLadder()).setHardness(0.4F).setStepSound(SoundType.LADDER).setUnlocalizedName("ladder"));
        registerBlock(66, "rail", (new BlockRail()).setHardness(0.7F).setStepSound(SoundType.METAL).setUnlocalizedName("rail"));
        registerBlock(67, "stone_stairs", (new BlockStairs(block.getDefaultState())).setUnlocalizedName("stairsStone"));
        registerBlock(68, "wall_sign", (new BlockWallSign()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("sign").disableStats());
        registerBlock(69, "lever", (new BlockLever()).setHardness(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("lever"));
        registerBlock(70, "stone_pressure_plate", (new BlockPressurePlate(Material.rock, BlockPressurePlate.Sensitivity.MOBS)).setHardness(0.5F).setStepSound(SoundType.STONE).setUnlocalizedName("pressurePlateStone"));
        registerBlock(71, "iron_door", (new BlockDoor(Material.iron)).setHardness(5.0F).setStepSound(SoundType.METAL).setUnlocalizedName("doorIron").disableStats());
        registerBlock(72, "wooden_pressure_plate", (new BlockPressurePlate(Material.wood, BlockPressurePlate.Sensitivity.EVERYTHING)).setHardness(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("pressurePlateWood"));
        registerBlock(73, "redstone_ore", (new BlockRedstoneOre(false)).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreRedstone").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(74, "lit_redstone_ore", (new BlockRedstoneOre(true)).setLightLevel(0.625F).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreRedstone"));
        registerBlock(75, "unlit_redstone_torch", (new BlockRedstoneTorch(false)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("notGate"));
        registerBlock(76, "redstone_torch", (new BlockRedstoneTorch(true)).setHardness(0.0F).setLightLevel(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("notGate").setCreativeTab(CreativeTabs.tabRedstone));
        registerBlock(77, "stone_button", (new BlockButtonStone()).setHardness(0.5F).setStepSound(SoundType.STONE).setUnlocalizedName("button"));
        registerBlock(78, "snow_layer", (new BlockSnow()).setHardness(0.1F).setStepSound(SoundType.SNOW).setUnlocalizedName("snow").setLightOpacity(0));
        registerBlock(79, "ice", (new BlockIce()).setHardness(0.5F).setLightOpacity(3).setStepSound(SoundType.GLASS).setUnlocalizedName("ice"));
        registerBlock(80, "snow", (new BlockSnowBlock()).setHardness(0.2F).setStepSound(SoundType.SNOW).setUnlocalizedName("snow"));
        registerBlock(81, "cactus", (new BlockCactus()).setHardness(0.4F).setStepSound(SoundType.CLOTH).setUnlocalizedName("cactus"));
        registerBlock(82, "clay", (new BlockClay()).setHardness(0.6F).setStepSound(SoundType.GROUND).setUnlocalizedName("clay"));
        registerBlock(83, "reeds", (new BlockReed()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("reeds").disableStats());
        registerBlock(84, "jukebox", (new BlockJukebox()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("jukebox"));
        registerBlock(85, "fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.OAK.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("fence"));
        Block block7 = (new BlockPumpkin()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("pumpkin");
        registerBlock(86, "pumpkin", block7);
        registerBlock(87, "netherrack", (new BlockNetherrack()).setHardness(0.4F).setStepSound(SoundType.STONE).setUnlocalizedName("hellrock"));
        registerBlock(88, "soul_sand", (new BlockSoulSand()).setHardness(0.5F).setStepSound(SoundType.SAND).setUnlocalizedName("hellsand"));
        registerBlock(89, "glowstone", (new BlockGlowstone(Material.glass)).setHardness(0.3F).setStepSound(SoundType.GLASS).setLightLevel(1.0F).setUnlocalizedName("lightgem"));
        registerBlock(90, "portal", (new BlockPortal()).setHardness(-1.0F).setStepSound(SoundType.GLASS).setLightLevel(0.75F).setUnlocalizedName("portal"));
        registerBlock(91, "lit_pumpkin", (new BlockPumpkin()).setHardness(1.0F).setStepSound(SoundType.WOOD).setLightLevel(1.0F).setUnlocalizedName("litpumpkin"));
        registerBlock(92, "cake", (new BlockCake()).setHardness(0.5F).setStepSound(SoundType.CLOTH).setUnlocalizedName("cake").disableStats());
        registerBlock(93, "unpowered_repeater", (new BlockRedstoneRepeater(false)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("diode").disableStats());
        registerBlock(94, "powered_repeater", (new BlockRedstoneRepeater(true)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("diode").disableStats());
        registerBlock(95, "stained_glass", (new BlockStainedGlass(Material.glass)).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("stainedGlass"));
        registerBlock(96, "trapdoor", (new BlockTrapDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("trapdoor").disableStats());
        registerBlock(97, "monster_egg", (new BlockSilverfish()).setHardness(0.75F).setUnlocalizedName("monsterStoneEgg"));
        Block block8 = (new BlockStoneBrick()).setHardness(1.5F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stonebricksmooth");
        registerBlock(98, "stonebrick", block8);
        registerBlock(99, "brown_mushroom_block", (new BlockHugeMushroom(Material.wood, MapColor.dirtColor, block3)).setHardness(0.2F).setStepSound(SoundType.WOOD).setUnlocalizedName("mushroom"));
        registerBlock(100, "red_mushroom_block", (new BlockHugeMushroom(Material.wood, MapColor.redColor, block4)).setHardness(0.2F).setStepSound(SoundType.WOOD).setUnlocalizedName("mushroom"));
        registerBlock(101, "iron_bars", (new BlockPane(Material.iron, true)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("fenceIron"));
        registerBlock(102, "glass_pane", (new BlockPane(Material.glass, false)).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("thinGlass"));
        Block block9 = (new BlockMelon()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("melon");
        registerBlock(103, "melon_block", block9);
        registerBlock(104, "pumpkin_stem", (new BlockStem(block7)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("pumpkinStem"));
        registerBlock(105, "melon_stem", (new BlockStem(block9)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("pumpkinStem"));
        registerBlock(106, "vine", (new BlockVine()).setHardness(0.2F).setStepSound(SoundType.PLANT).setUnlocalizedName("vine"));
        registerBlock(107, "fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.OAK)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("fenceGate"));
        registerBlock(108, "brick_stairs", (new BlockStairs(block5.getDefaultState())).setUnlocalizedName("stairsBrick"));
        registerBlock(109, "stone_brick_stairs", (new BlockStairs(block8.getDefaultState().withProperty(BlockStoneBrick.VARIANT, BlockStoneBrick.EnumType.DEFAULT))).setUnlocalizedName("stairsStoneBrickSmooth"));
        registerBlock(110, "mycelium", (new BlockMycelium()).setHardness(0.6F).setStepSound(SoundType.PLANT).setUnlocalizedName("mycel"));
        registerBlock(111, "waterlily", (new BlockLilyPad()).setHardness(0.0F).setStepSound(SoundType.PLANT).setUnlocalizedName("waterlily"));
        Block block10 = (new BlockNetherBrick()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("netherBrick").setCreativeTab(CreativeTabs.tabBlock);
        registerBlock(112, "nether_brick", block10);
        registerBlock(113, "nether_brick_fence", (new BlockFence(Material.rock, MapColor.netherrackColor)).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("netherFence"));
        registerBlock(114, "nether_brick_stairs", (new BlockStairs(block10.getDefaultState())).setUnlocalizedName("stairsNetherBrick"));
        registerBlock(115, "nether_wart", (new BlockNetherWart()).setUnlocalizedName("netherStalk"));
        registerBlock(116, "enchanting_table", (new BlockEnchantmentTable()).setHardness(5.0F).setResistance(2000.0F).setUnlocalizedName("enchantmentTable"));
        registerBlock(117, "brewing_stand", (new BlockBrewingStand()).setHardness(0.5F).setLightLevel(0.125F).setUnlocalizedName("brewingStand"));
        registerBlock(118, "cauldron", (new BlockCauldron()).setHardness(2.0F).setUnlocalizedName("cauldron"));
        registerBlock(119, "end_portal", (new BlockEndPortal(Material.portal)).setHardness(-1.0F).setResistance(6000000.0F));
        registerBlock(120, "end_portal_frame", (new BlockEndPortalFrame()).setStepSound(SoundType.GLASS).setLightLevel(0.125F).setHardness(-1.0F).setUnlocalizedName("endPortalFrame").setResistance(6000000.0F).setCreativeTab(CreativeTabs.tabDecorations));
        registerBlock(121, "end_stone", (new Block(Material.rock, MapColor.sandColor)).setHardness(3.0F).setResistance(15.0F).setStepSound(SoundType.STONE).setUnlocalizedName("whiteStone").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(122, "dragon_egg", (new BlockDragonEgg()).setHardness(3.0F).setResistance(15.0F).setStepSound(SoundType.STONE).setLightLevel(0.125F).setUnlocalizedName("dragonEgg"));
        registerBlock(123, "redstone_lamp", (new BlockRedstoneLight(false)).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("redstoneLight").setCreativeTab(CreativeTabs.tabRedstone));
        registerBlock(124, "lit_redstone_lamp", (new BlockRedstoneLight(true)).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("redstoneLight"));
        registerBlock(125, "double_wooden_slab", (new BlockDoubleWoodSlab()).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("woodSlab"));
        registerBlock(126, "wooden_slab", (new BlockHalfWoodSlab()).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("woodSlab"));
        registerBlock(127, "cocoa", (new BlockCocoa()).setHardness(0.2F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("cocoa"));
        registerBlock(128, "sandstone_stairs", (new BlockStairs(block2.getDefaultState().withProperty(BlockSandStone.TYPE, BlockSandStone.EnumType.SMOOTH))).setUnlocalizedName("stairsSandStone"));
        registerBlock(129, "emerald_ore", (new BlockOre()).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("oreEmerald"));
        registerBlock(130, "ender_chest", (new BlockEnderChest()).setHardness(22.5F).setResistance(1000.0F).setStepSound(SoundType.STONE).setUnlocalizedName("enderChest").setLightLevel(0.5F));
        registerBlock(131, "tripwire_hook", (new BlockTripWireHook()).setUnlocalizedName("tripWireSource"));
        registerBlock(132, "tripwire", (new BlockTripWire()).setUnlocalizedName("tripWire"));
        registerBlock(133, "emerald_block", (new Block(Material.iron, MapColor.emeraldColor)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("blockEmerald").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(134, "spruce_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.SPRUCE))).setUnlocalizedName("stairsWoodSpruce"));
        registerBlock(135, "birch_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.BIRCH))).setUnlocalizedName("stairsWoodBirch"));
        registerBlock(136, "jungle_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.JUNGLE))).setUnlocalizedName("stairsWoodJungle"));
        registerBlock(137, "command_block", (new BlockCommandBlock(MapColor.brownColor)).setBlockUnbreakable().setResistance(6000000.0F).setUnlocalizedName("commandBlock"));
        registerBlock(138, "beacon", (new BlockBeacon()).setUnlocalizedName("beacon").setLightLevel(1.0F));
        registerBlock(139, "cobblestone_wall", (new BlockWall(block)).setUnlocalizedName("cobbleWall"));
        registerBlock(140, "flower_pot", (new BlockFlowerPot()).setHardness(0.0F).setStepSound(SoundType.STONE).setUnlocalizedName("flowerPot"));
        registerBlock(141, "carrots", (new BlockCarrot()).setUnlocalizedName("carrots"));
        registerBlock(142, "potatoes", (new BlockPotato()).setUnlocalizedName("potatoes"));
        registerBlock(143, "wooden_button", (new BlockButtonWood()).setHardness(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("button"));
        registerBlock(144, "skull", (new BlockSkull()).setHardness(1.0F).setStepSound(SoundType.STONE).setUnlocalizedName("skull"));
        registerBlock(145, "anvil", (new BlockAnvil()).setHardness(5.0F).setStepSound(SoundType.ANVIL).setResistance(2000.0F).setUnlocalizedName("anvil"));
        registerBlock(146, "trapped_chest", (new BlockChest(BlockChest.Type.TRAP)).setHardness(2.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("chestTrap"));
        registerBlock(147, "light_weighted_pressure_plate", (new BlockPressurePlateWeighted(Material.iron, 15, MapColor.goldColor)).setHardness(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("weightedPlate_light"));
        registerBlock(148, "heavy_weighted_pressure_plate", (new BlockPressurePlateWeighted(Material.iron, 150)).setHardness(0.5F).setStepSound(SoundType.WOOD).setUnlocalizedName("weightedPlate_heavy"));
        registerBlock(149, "unpowered_comparator", (new BlockRedstoneComparator(false)).setHardness(0.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("comparator").disableStats());
        registerBlock(150, "powered_comparator", (new BlockRedstoneComparator(true)).setHardness(0.0F).setLightLevel(0.625F).setStepSound(SoundType.WOOD).setUnlocalizedName("comparator").disableStats());
        registerBlock(151, "daylight_detector", new BlockDaylightDetector(false));
        registerBlock(152, "redstone_block", (new BlockCompressedPowered(Material.iron, MapColor.tntColor)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.METAL).setUnlocalizedName("blockRedstone").setCreativeTab(CreativeTabs.tabRedstone));
        registerBlock(153, "quartz_ore", (new BlockOre(MapColor.netherrackColor)).setHardness(3.0F).setResistance(5.0F).setStepSound(SoundType.STONE).setUnlocalizedName("netherquartz"));
        registerBlock(154, "hopper", (new BlockHopper()).setHardness(3.0F).setResistance(8.0F).setStepSound(SoundType.METAL).setUnlocalizedName("hopper"));
        Block block11 = (new BlockQuartz()).setStepSound(SoundType.STONE).setHardness(0.8F).setUnlocalizedName("quartzBlock");
        registerBlock(155, "quartz_block", block11);
        registerBlock(156, "quartz_stairs", (new BlockStairs(block11.getDefaultState().withProperty(BlockQuartz.VARIANT, BlockQuartz.EnumType.DEFAULT))).setUnlocalizedName("stairsQuartz"));
        registerBlock(157, "activator_rail", (new BlockRailPowered()).setHardness(0.7F).setStepSound(SoundType.METAL).setUnlocalizedName("activatorRail"));
        registerBlock(158, "dropper", (new BlockDropper()).setHardness(3.5F).setStepSound(SoundType.STONE).setUnlocalizedName("dropper"));
        registerBlock(159, "stained_hardened_clay", (new BlockColored(Material.rock)).setHardness(1.25F).setResistance(7.0F).setStepSound(SoundType.STONE).setUnlocalizedName("clayHardenedStained"));
        registerBlock(160, "stained_glass_pane", (new BlockStainedGlassPane()).setHardness(0.3F).setStepSound(SoundType.GLASS).setUnlocalizedName("thinStainedGlass"));
        registerBlock(161, "leaves2", (new BlockNewLeaf()).setUnlocalizedName("leaves"));
        registerBlock(162, "log2", (new BlockNewLog()).setUnlocalizedName("log"));
        registerBlock(163, "acacia_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.ACACIA))).setUnlocalizedName("stairsWoodAcacia"));
        registerBlock(164, "dark_oak_stairs", (new BlockStairs(block1.getDefaultState().withProperty(BlockPlanks.VARIANT, BlockPlanks.EnumType.DARK_OAK))).setUnlocalizedName("stairsWoodDarkOak"));
        registerBlock(165, "slime", (new BlockSlime()).setUnlocalizedName("slime").setStepSound(SoundType.SLIME));
        registerBlock(166, "barrier", (new BlockBarrier()).setUnlocalizedName("barrier"));
        registerBlock(167, "iron_trapdoor", (new BlockTrapDoor(Material.iron)).setHardness(5.0F).setStepSound(SoundType.METAL).setUnlocalizedName("ironTrapdoor").disableStats());
        registerBlock(168, "prismarine", (new BlockPrismarine()).setHardness(1.5F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("prismarine"));
        registerBlock(169, "sea_lantern", (new BlockSeaLantern(Material.glass)).setHardness(0.3F).setStepSound(SoundType.GLASS).setLightLevel(1.0F).setUnlocalizedName("seaLantern"));
        registerBlock(170, "hay_block", (new BlockHay()).setHardness(0.5F).setStepSound(SoundType.PLANT).setUnlocalizedName("hayBlock").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(171, "carpet", (new BlockCarpet()).setHardness(0.1F).setStepSound(SoundType.CLOTH).setUnlocalizedName("woolCarpet").setLightOpacity(0));
        registerBlock(172, "hardened_clay", (new BlockHardenedClay()).setHardness(1.25F).setResistance(7.0F).setStepSound(SoundType.STONE).setUnlocalizedName("clayHardened"));
        registerBlock(173, "coal_block", (new Block(Material.rock, MapColor.blackColor)).setHardness(5.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("blockCoal").setCreativeTab(CreativeTabs.tabBlock));
        registerBlock(174, "packed_ice", (new BlockPackedIce()).setHardness(0.5F).setStepSound(SoundType.GLASS).setUnlocalizedName("icePacked"));
        registerBlock(175, "double_plant", new BlockDoublePlant());
        registerBlock(176, "standing_banner", (new BlockBanner.BlockBannerStanding()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("banner").disableStats());
        registerBlock(177, "wall_banner", (new BlockBanner.BlockBannerHanging()).setHardness(1.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("banner").disableStats());
        registerBlock(178, "daylight_detector_inverted", new BlockDaylightDetector(true));
        Block block12 = (new BlockRedSandstone()).setStepSound(SoundType.STONE).setHardness(0.8F).setUnlocalizedName("redSandStone");
        registerBlock(179, "red_sandstone", block12);
        registerBlock(180, "red_sandstone_stairs", (new BlockStairs(block12.getDefaultState().withProperty(BlockRedSandstone.TYPE, BlockRedSandstone.EnumType.SMOOTH))).setUnlocalizedName("stairsRedSandStone"));
        registerBlock(181, "double_stone_slab2", (new BlockDoubleStoneSlabNew()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stoneSlab2"));
        registerBlock(182, "stone_slab2", (new BlockHalfStoneSlabNew()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("stoneSlab2"));
        registerBlock(183, "spruce_fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.SPRUCE)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("spruceFenceGate"));
        registerBlock(184, "birch_fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.BIRCH)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("birchFenceGate"));
        registerBlock(185, "jungle_fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.JUNGLE)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("jungleFenceGate"));
        registerBlock(186, "dark_oak_fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.DARK_OAK)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("darkOakFenceGate"));
        registerBlock(187, "acacia_fence_gate", (new BlockFenceGate(BlockPlanks.EnumType.ACACIA)).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("acaciaFenceGate"));
        registerBlock(188, "spruce_fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.SPRUCE.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("spruceFence"));
        registerBlock(189, "birch_fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.BIRCH.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("birchFence"));
        registerBlock(190, "jungle_fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.JUNGLE.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("jungleFence"));
        registerBlock(191, "dark_oak_fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.DARK_OAK.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("darkOakFence"));
        registerBlock(192, "acacia_fence", (new BlockFence(Material.wood, BlockPlanks.EnumType.ACACIA.getMapColor())).setHardness(2.0F).setResistance(5.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("acaciaFence"));
        registerBlock(193, "spruce_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorSpruce").disableStats());
        registerBlock(194, "birch_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorBirch").disableStats());
        registerBlock(195, "jungle_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorJungle").disableStats());
        registerBlock(196, "acacia_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorAcacia").disableStats());
        registerBlock(197, "dark_oak_door", (new BlockDoor(Material.wood)).setHardness(3.0F).setStepSound(SoundType.WOOD).setUnlocalizedName("doorDarkOak").disableStats());
        registerBlock(198, "end_rod", (new BlockEndRod()).setHardness(0.0F).setLightLevel(0.9375F).setStepSound(SoundType.WOOD).setUnlocalizedName("endRod"));
        registerBlock(199, "chorus_plant", (new BlockChorusPlant()).setHardness(0.4F).setStepSound(SoundType.WOOD).setUnlocalizedName("chorusPlant"));
        registerBlock(200, "chorus_flower", (new BlockChorusFlower()).setHardness(0.4F).setStepSound(SoundType.WOOD).setUnlocalizedName("chorusFlower"));
        Block block13 = (new Block(Material.rock)).setHardness(1.5F).setResistance(10.0F).setStepSound(SoundType.STONE).setCreativeTab(CreativeTabs.tabBlock).setUnlocalizedName("purpurBlock");
        registerBlock(201, "purpur_block", block13);
        registerBlock(202, "purpur_pillar", (new BlockRotatedPillar(Material.rock)).setHardness(1.5F).setResistance(10.0F).setStepSound(SoundType.STONE).setCreativeTab(CreativeTabs.tabBlock).setUnlocalizedName("purpurPillar"));
        registerBlock(203, "purpur_stairs", (new BlockStairs(block13.getDefaultState())).setUnlocalizedName("stairsPurpur"));
        registerBlock(204, "purpur_double_slab", (new BlockPurpurSlab.Double()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("purpurSlab"));
        registerBlock(205, "purpur_slab", (new BlockPurpurSlab.Half()).setHardness(2.0F).setResistance(10.0F).setStepSound(SoundType.STONE).setUnlocalizedName("purpurSlab"));
        registerBlock(206, "end_bricks", (new Block(Material.rock)).setStepSound(SoundType.STONE).setHardness(0.8F).setCreativeTab(CreativeTabs.tabBlock).setUnlocalizedName("endBricks"));
        registerBlock(207, "beetroots", (new BlockBeetroot()).setUnlocalizedName("beetroots"));
        Block block14 = (new BlockGrassPath()).setHardness(0.65F).setStepSound(SoundType.PLANT).setUnlocalizedName("grassPath").disableStats();
        registerBlock(208, "grass_path", block14);
        registerBlock(209, "end_gateway", (new BlockEndGateway(Material.portal)).setHardness(-1.0F).setResistance(6000000.0F));
        registerBlock(210, "repeating_command_block", (new BlockCommandBlock(MapColor.purpleColor)).setBlockUnbreakable().setResistance(6000000.0F).setUnlocalizedName("repeatingCommandBlock"));
        registerBlock(211, "chain_command_block", (new BlockCommandBlock(MapColor.greenColor)).setBlockUnbreakable().setResistance(6000000.0F).setUnlocalizedName("chainCommandBlock"));
        registerBlock(212, "frosted_ice", (new BlockFrostedIce()).setHardness(0.5F).setLightOpacity(3).setStepSound(SoundType.GLASS).setUnlocalizedName("frostedIce"));
        registerBlock(255, "structure_block", (new BlockStructure()).setBlockUnbreakable().setResistance(6000000.0F).setUnlocalizedName("structureBlock").setLightLevel(1.0F));
        blockRegistry.validateKey();

        for (Block block15 : blockRegistry)
        {
            if (block15.blockMaterial == Material.air)
            {
                block15.useNeighborBrightness = false;
            }
            else
            {
                boolean flag = false;
                boolean flag1 = block15 instanceof BlockStairs;
                boolean flag2 = block15 instanceof BlockSlab;
                boolean flag3 = block15 == block6 || block15 == block14;
                boolean flag4 = block15.translucent;
                boolean flag5 = block15.lightOpacity == 0;

                if (flag1 || flag2 || flag3 || flag4 || flag5)
                {
                    flag = true;
                }

                block15.useNeighborBrightness = flag;
            }
        }

        Set<Block> set = Sets.newHashSet(new Block[] {(Block)blockRegistry.getObject(new ResourceLocation("tripwire"))});

        for (Block block16 : blockRegistry)
        {
            if (set.contains(block16))
            {
                for (int i = 0; i < 15; ++i)
                {
                    int j = blockRegistry.getIDForObject(block16) << 4 | i;
                    BLOCK_STATE_IDS.put(block16.getStateFromMeta(i), j);
                }
            }
            else
            {
                for (IBlockState iblockstate : block16.getBlockState().getValidStates())
                {
                    int k = blockRegistry.getIDForObject(block16) << 4 | block16.getMetaFromState(iblockstate);
                    BLOCK_STATE_IDS.put(iblockstate, k);
                }
            }
        }
    }

    private static void registerBlock(int id, ResourceLocation textualID, Block block_)
    {
        blockRegistry.register(id, textualID, block_);
    }

    private static void registerBlock(int id, String textualID, Block block_)
    {
        registerBlock(id, new ResourceLocation(textualID), block_);
    }

    public static enum EnumOffsetType
    {
        NONE,
        XZ,
        XYZ;
    }
}
