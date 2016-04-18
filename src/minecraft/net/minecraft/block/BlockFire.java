package net.minecraft.block;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.BlockRenderLayer;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;
import net.minecraft.world.WorldProviderEnd;

public class BlockFire extends Block
{
    public static final PropertyInteger AGE = PropertyInteger.create("age", 0, 15);
    public static final PropertyBool NORTH = PropertyBool.create("north");
    public static final PropertyBool EAST = PropertyBool.create("east");
    public static final PropertyBool SOUTH = PropertyBool.create("south");
    public static final PropertyBool WEST = PropertyBool.create("west");
    public static final PropertyBool UPPER = PropertyBool.create("up");
    private final Map<Block, Integer> encouragements = Maps.<Block, Integer>newIdentityHashMap();
    private final Map<Block, Integer> flammabilities = Maps.<Block, Integer>newIdentityHashMap();

    /**
     * Get the actual Block state of this Block at the given position. This applies properties not visible in the
     * metadata, such as fence connections.
     */
    public IBlockState getActualState(IBlockState state, IBlockAccess worldIn, BlockPos pos)
    {
        return !worldIn.getBlockState(pos.down()).func_185896_q() && !Blocks.fire.canCatchFire(worldIn, pos.down()) ? state.withProperty(NORTH, Boolean.valueOf(this.canCatchFire(worldIn, pos.north()))).withProperty(EAST, Boolean.valueOf(this.canCatchFire(worldIn, pos.east()))).withProperty(SOUTH, Boolean.valueOf(this.canCatchFire(worldIn, pos.south()))).withProperty(WEST, Boolean.valueOf(this.canCatchFire(worldIn, pos.west()))).withProperty(UPPER, Boolean.valueOf(this.canCatchFire(worldIn, pos.up()))) : this.getDefaultState();
    }

    protected BlockFire()
    {
        super(Material.fire);
        this.setDefaultState(this.blockState.getBaseState().withProperty(AGE, Integer.valueOf(0)).withProperty(NORTH, Boolean.valueOf(false)).withProperty(EAST, Boolean.valueOf(false)).withProperty(SOUTH, Boolean.valueOf(false)).withProperty(WEST, Boolean.valueOf(false)).withProperty(UPPER, Boolean.valueOf(false)));
        this.setTickRandomly(true);
    }

    public static void init()
    {
        Blocks.fire.setFireInfo(Blocks.planks, 5, 20);
        Blocks.fire.setFireInfo(Blocks.double_wooden_slab, 5, 20);
        Blocks.fire.setFireInfo(Blocks.wooden_slab, 5, 20);
        Blocks.fire.setFireInfo(Blocks.oak_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.spruce_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.birch_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.jungle_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.dark_oak_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.acacia_fence_gate, 5, 20);
        Blocks.fire.setFireInfo(Blocks.oak_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.spruce_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.birch_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.jungle_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.dark_oak_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.acacia_fence, 5, 20);
        Blocks.fire.setFireInfo(Blocks.oak_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.birch_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.spruce_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.jungle_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.acacia_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.dark_oak_stairs, 5, 20);
        Blocks.fire.setFireInfo(Blocks.log, 5, 5);
        Blocks.fire.setFireInfo(Blocks.log2, 5, 5);
        Blocks.fire.setFireInfo(Blocks.leaves, 30, 60);
        Blocks.fire.setFireInfo(Blocks.leaves2, 30, 60);
        Blocks.fire.setFireInfo(Blocks.bookshelf, 30, 20);
        Blocks.fire.setFireInfo(Blocks.tnt, 15, 100);
        Blocks.fire.setFireInfo(Blocks.tallgrass, 60, 100);
        Blocks.fire.setFireInfo(Blocks.double_plant, 60, 100);
        Blocks.fire.setFireInfo(Blocks.yellow_flower, 60, 100);
        Blocks.fire.setFireInfo(Blocks.red_flower, 60, 100);
        Blocks.fire.setFireInfo(Blocks.deadbush, 60, 100);
        Blocks.fire.setFireInfo(Blocks.wool, 30, 60);
        Blocks.fire.setFireInfo(Blocks.vine, 15, 100);
        Blocks.fire.setFireInfo(Blocks.coal_block, 5, 5);
        Blocks.fire.setFireInfo(Blocks.hay_block, 60, 20);
        Blocks.fire.setFireInfo(Blocks.carpet, 60, 20);
    }

    public void setFireInfo(Block blockIn, int encouragement, int flammability)
    {
        this.encouragements.put(blockIn, Integer.valueOf(encouragement));
        this.flammabilities.put(blockIn, Integer.valueOf(flammability));
    }

    public AxisAlignedBB getSelectedBoundingBox(IBlockState worldIn, World pos, BlockPos p_180646_3_)
    {
        return NULL_AABB;
    }

    /**
     * Used to determine ambient occlusion and culling when rebuilding chunks for render
     */
    public boolean isOpaqueCube(IBlockState p_149662_1_)
    {
        return false;
    }

    public boolean isFullCube(IBlockState state)
    {
        return false;
    }

    /**
     * Returns the quantity of items to drop on block destruction.
     */
    public int quantityDropped(Random random)
    {
        return 0;
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 30;
    }

    public void updateTick(World worldIn, BlockPos pos, IBlockState state, Random rand)
    {
        if (worldIn.getGameRules().getBoolean("doFireTick"))
        {
            if (!this.canPlaceBlockAt(worldIn, pos))
            {
                worldIn.setBlockToAir(pos);
            }

            Block block = worldIn.getBlockState(pos.down()).getBlock();
            boolean flag = block == Blocks.netherrack;

            if (worldIn.provider instanceof WorldProviderEnd && block == Blocks.bedrock)
            {
                flag = true;
            }

            int i = ((Integer)state.getValue(AGE)).intValue();

            if (!flag && worldIn.isRaining() && this.canDie(worldIn, pos) && rand.nextFloat() < 0.2F + (float)i * 0.03F)
            {
                worldIn.setBlockToAir(pos);
            }
            else
            {
                if (i < 15)
                {
                    state = state.withProperty(AGE, Integer.valueOf(i + rand.nextInt(3) / 2));
                    worldIn.setBlockState(pos, state, 4);
                }

                worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn) + rand.nextInt(10));

                if (!flag)
                {
                    if (!this.canNeighborCatchFire(worldIn, pos))
                    {
                        if (!worldIn.getBlockState(pos.down()).func_185896_q() || i > 3)
                        {
                            worldIn.setBlockToAir(pos);
                        }

                        return;
                    }

                    if (!this.canCatchFire(worldIn, pos.down()) && i == 15 && rand.nextInt(4) == 0)
                    {
                        worldIn.setBlockToAir(pos);
                        return;
                    }
                }

                boolean flag1 = worldIn.isBlockinHighHumidity(pos);
                int j = 0;

                if (flag1)
                {
                    j = -50;
                }

                this.catchOnFire(worldIn, pos.east(), 300 + j, rand, i);
                this.catchOnFire(worldIn, pos.west(), 300 + j, rand, i);
                this.catchOnFire(worldIn, pos.down(), 250 + j, rand, i);
                this.catchOnFire(worldIn, pos.up(), 250 + j, rand, i);
                this.catchOnFire(worldIn, pos.north(), 300 + j, rand, i);
                this.catchOnFire(worldIn, pos.south(), 300 + j, rand, i);

                for (int k = -1; k <= 1; ++k)
                {
                    for (int l = -1; l <= 1; ++l)
                    {
                        for (int i1 = -1; i1 <= 4; ++i1)
                        {
                            if (k != 0 || i1 != 0 || l != 0)
                            {
                                int j1 = 100;

                                if (i1 > 1)
                                {
                                    j1 += (i1 - 1) * 100;
                                }

                                BlockPos blockpos = pos.add(k, i1, l);
                                int k1 = this.getNeighborEncouragement(worldIn, blockpos);

                                if (k1 > 0)
                                {
                                    int l1 = (k1 + 40 + worldIn.getDifficulty().getDifficultyId() * 7) / (i + 30);

                                    if (flag1)
                                    {
                                        l1 /= 2;
                                    }

                                    if (l1 > 0 && rand.nextInt(j1) <= l1 && (!worldIn.isRaining() || !this.canDie(worldIn, blockpos)))
                                    {
                                        int i2 = i + rand.nextInt(5) / 4;

                                        if (i2 > 15)
                                        {
                                            i2 = 15;
                                        }

                                        worldIn.setBlockState(blockpos, state.withProperty(AGE, Integer.valueOf(i2)), 3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    protected boolean canDie(World worldIn, BlockPos pos)
    {
        return worldIn.isRainingAt(pos) || worldIn.isRainingAt(pos.west()) || worldIn.isRainingAt(pos.east()) || worldIn.isRainingAt(pos.north()) || worldIn.isRainingAt(pos.south());
    }

    public boolean requiresUpdates()
    {
        return false;
    }

    private int getFlammability(Block blockIn)
    {
        Integer integer = (Integer)this.flammabilities.get(blockIn);
        return integer == null ? 0 : integer.intValue();
    }

    private int getEncouragement(Block blockIn)
    {
        Integer integer = (Integer)this.encouragements.get(blockIn);
        return integer == null ? 0 : integer.intValue();
    }

    private void catchOnFire(World worldIn, BlockPos pos, int chance, Random random, int age)
    {
        int i = this.getFlammability(worldIn.getBlockState(pos).getBlock());

        if (random.nextInt(chance) < i)
        {
            IBlockState iblockstate = worldIn.getBlockState(pos);

            if (random.nextInt(age + 10) < 5 && !worldIn.isRainingAt(pos))
            {
                int j = age + random.nextInt(5) / 4;

                if (j > 15)
                {
                    j = 15;
                }

                worldIn.setBlockState(pos, this.getDefaultState().withProperty(AGE, Integer.valueOf(j)), 3);
            }
            else
            {
                worldIn.setBlockToAir(pos);
            }

            if (iblockstate.getBlock() == Blocks.tnt)
            {
                Blocks.tnt.onBlockDestroyedByPlayer(worldIn, pos, iblockstate.withProperty(BlockTNT.EXPLODE, Boolean.valueOf(true)));
            }
        }
    }

    private boolean canNeighborCatchFire(World worldIn, BlockPos pos)
    {
        for (EnumFacing enumfacing : EnumFacing.values())
        {
            if (this.canCatchFire(worldIn, pos.offset(enumfacing)))
            {
                return true;
            }
        }

        return false;
    }

    private int getNeighborEncouragement(World worldIn, BlockPos pos)
    {
        if (!worldIn.isAirBlock(pos))
        {
            return 0;
        }
        else
        {
            int i = 0;

            for (EnumFacing enumfacing : EnumFacing.values())
            {
                i = Math.max(this.getEncouragement(worldIn.getBlockState(pos.offset(enumfacing)).getBlock()), i);
            }

            return i;
        }
    }

    /**
     * Returns if this block is collidable (only used by Fire). Args: x, y, z
     */
    public boolean isCollidable()
    {
        return false;
    }

    /**
     * Checks if the block can be caught on fire
     */
    public boolean canCatchFire(IBlockAccess worldIn, BlockPos pos)
    {
        return this.getEncouragement(worldIn.getBlockState(pos).getBlock()) > 0;
    }

    public boolean canPlaceBlockAt(World worldIn, BlockPos pos)
    {
        return worldIn.getBlockState(pos.down()).func_185896_q() || this.canNeighborCatchFire(worldIn, pos);
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (!worldIn.getBlockState(pos.down()).func_185896_q() && !this.canNeighborCatchFire(worldIn, pos))
        {
            worldIn.setBlockToAir(pos);
        }
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
        if (worldIn.provider.getDimensionType().getId() > 0 || !Blocks.portal.func_176548_d(worldIn, pos))
        {
            if (!worldIn.getBlockState(pos.down()).func_185896_q() && !this.canNeighborCatchFire(worldIn, pos))
            {
                worldIn.setBlockToAir(pos);
            }
            else
            {
                worldIn.scheduleUpdate(pos, this, this.tickRate(worldIn) + worldIn.rand.nextInt(10));
            }
        }
    }

    public void randomDisplayTick(IBlockState worldIn, World pos, BlockPos state, Random rand)
    {
        if (rand.nextInt(24) == 0)
        {
            pos.func_184134_a((double)((float)state.getX() + 0.5F), (double)((float)state.getY() + 0.5F), (double)((float)state.getZ() + 0.5F), SoundEvents.block_fire_ambient, SoundCategory.BLOCKS, 1.0F + rand.nextFloat(), rand.nextFloat() * 0.7F + 0.3F, false);
        }

        if (!pos.getBlockState(state.down()).func_185896_q() && !Blocks.fire.canCatchFire(pos, state.down()))
        {
            if (Blocks.fire.canCatchFire(pos, state.west()))
            {
                for (int j = 0; j < 2; ++j)
                {
                    double d3 = (double)state.getX() + rand.nextDouble() * 0.10000000149011612D;
                    double d8 = (double)state.getY() + rand.nextDouble();
                    double d13 = (double)state.getZ() + rand.nextDouble();
                    pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d3, d8, d13, 0.0D, 0.0D, 0.0D, new int[0]);
                }
            }

            if (Blocks.fire.canCatchFire(pos, state.east()))
            {
                for (int k = 0; k < 2; ++k)
                {
                    double d4 = (double)(state.getX() + 1) - rand.nextDouble() * 0.10000000149011612D;
                    double d9 = (double)state.getY() + rand.nextDouble();
                    double d14 = (double)state.getZ() + rand.nextDouble();
                    pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d4, d9, d14, 0.0D, 0.0D, 0.0D, new int[0]);
                }
            }

            if (Blocks.fire.canCatchFire(pos, state.north()))
            {
                for (int l = 0; l < 2; ++l)
                {
                    double d5 = (double)state.getX() + rand.nextDouble();
                    double d10 = (double)state.getY() + rand.nextDouble();
                    double d15 = (double)state.getZ() + rand.nextDouble() * 0.10000000149011612D;
                    pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d5, d10, d15, 0.0D, 0.0D, 0.0D, new int[0]);
                }
            }

            if (Blocks.fire.canCatchFire(pos, state.south()))
            {
                for (int i1 = 0; i1 < 2; ++i1)
                {
                    double d6 = (double)state.getX() + rand.nextDouble();
                    double d11 = (double)state.getY() + rand.nextDouble();
                    double d16 = (double)(state.getZ() + 1) - rand.nextDouble() * 0.10000000149011612D;
                    pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d6, d11, d16, 0.0D, 0.0D, 0.0D, new int[0]);
                }
            }

            if (Blocks.fire.canCatchFire(pos, state.up()))
            {
                for (int j1 = 0; j1 < 2; ++j1)
                {
                    double d7 = (double)state.getX() + rand.nextDouble();
                    double d12 = (double)(state.getY() + 1) - rand.nextDouble() * 0.10000000149011612D;
                    double d17 = (double)state.getZ() + rand.nextDouble();
                    pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d7, d12, d17, 0.0D, 0.0D, 0.0D, new int[0]);
                }
            }
        }
        else
        {
            for (int i = 0; i < 3; ++i)
            {
                double d0 = (double)state.getX() + rand.nextDouble();
                double d1 = (double)state.getY() + rand.nextDouble() * 0.5D + 0.5D;
                double d2 = (double)state.getZ() + rand.nextDouble();
                pos.spawnParticle(EnumParticleTypes.SMOKE_LARGE, d0, d1, d2, 0.0D, 0.0D, 0.0D, new int[0]);
            }
        }
    }

    /**
     * Get the MapColor for this Block and the given BlockState
     */
    public MapColor getMapColor(IBlockState state)
    {
        return MapColor.tntColor;
    }

    public BlockRenderLayer getBlockLayer()
    {
        return BlockRenderLayer.CUTOUT;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(AGE, Integer.valueOf(meta));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Integer)state.getValue(AGE)).intValue();
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {AGE, NORTH, EAST, SOUTH, WEST, UPPER});
    }
}
