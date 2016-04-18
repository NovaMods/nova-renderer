package net.minecraft.block;

import java.util.List;
import java.util.Random;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.init.Items;
import net.minecraft.init.PotionTypes;
import net.minecraft.item.Item;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemBanner;
import net.minecraft.item.ItemStack;
import net.minecraft.potion.PotionUtils;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.TileEntityBanner;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockCauldron extends Block
{
    public static final PropertyInteger LEVEL = PropertyInteger.create("level", 0, 3);
    protected static final AxisAlignedBB field_185596_b = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 0.3125D, 1.0D);
    protected static final AxisAlignedBB field_185597_c = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 1.0D, 1.0D, 0.125D);
    protected static final AxisAlignedBB field_185598_d = new AxisAlignedBB(0.0D, 0.0D, 0.875D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185599_e = new AxisAlignedBB(0.875D, 0.0D, 0.0D, 1.0D, 1.0D, 1.0D);
    protected static final AxisAlignedBB field_185600_f = new AxisAlignedBB(0.0D, 0.0D, 0.0D, 0.125D, 1.0D, 1.0D);

    public BlockCauldron()
    {
        super(Material.iron, MapColor.stoneColor);
        this.setDefaultState(this.blockState.getBaseState().withProperty(LEVEL, Integer.valueOf(0)));
    }

    public void func_185477_a(IBlockState state, World worldIn, BlockPos pos, AxisAlignedBB p_185477_4_, List<AxisAlignedBB> p_185477_5_, Entity p_185477_6_)
    {
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185596_b);
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185600_f);
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185597_c);
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185599_e);
        func_185492_a(pos, p_185477_4_, p_185477_5_, field_185598_d);
    }

    public AxisAlignedBB func_185496_a(IBlockState state, IBlockAccess source, BlockPos pos)
    {
        return FULL_BLOCK_AABB;
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
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        int i = ((Integer)state.getValue(LEVEL)).intValue();
        float f = (float)pos.getY() + (6.0F + (float)(3 * i)) / 16.0F;

        if (!worldIn.isRemote && entityIn.isBurning() && i > 0 && entityIn.getEntityBoundingBox().minY <= (double)f)
        {
            entityIn.extinguish();
            this.setWaterLevel(worldIn, pos, state, i - 1);
        }
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (hitX == null)
        {
            return true;
        }
        else
        {
            int i = ((Integer)state.getValue(LEVEL)).intValue();
            Item item = hitX.getItem();

            if (item == Items.water_bucket)
            {
                if (i < 3 && !worldIn.isRemote)
                {
                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        playerIn.setHeldItem(side, new ItemStack(Items.bucket));
                    }

                    playerIn.triggerAchievement(StatList.field_188077_K);
                    this.setWaterLevel(worldIn, pos, state, 3);
                }

                return true;
            }
            else if (item == Items.bucket)
            {
                if (i == 3 && !worldIn.isRemote)
                {
                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        --hitX.stackSize;

                        if (hitX.stackSize == 0)
                        {
                            playerIn.setHeldItem(side, new ItemStack(Items.water_bucket));
                        }
                        else if (!playerIn.inventory.addItemStackToInventory(new ItemStack(Items.water_bucket)))
                        {
                            playerIn.dropPlayerItemWithRandomChoice(new ItemStack(Items.water_bucket), false);
                        }
                    }

                    playerIn.triggerAchievement(StatList.field_188078_L);
                    this.setWaterLevel(worldIn, pos, state, 0);
                }

                return true;
            }
            else if (item == Items.glass_bottle)
            {
                if (i > 0 && !worldIn.isRemote)
                {
                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        ItemStack itemstack1 = PotionUtils.func_185188_a(new ItemStack(Items.potionitem), PotionTypes.water);
                        playerIn.triggerAchievement(StatList.field_188078_L);

                        if (--hitX.stackSize == 0)
                        {
                            playerIn.setHeldItem(side, itemstack1);
                        }
                        else if (!playerIn.inventory.addItemStackToInventory(itemstack1))
                        {
                            playerIn.dropPlayerItemWithRandomChoice(itemstack1, false);
                        }
                        else if (playerIn instanceof EntityPlayerMP)
                        {
                            ((EntityPlayerMP)playerIn).sendContainerToPlayer(playerIn.inventoryContainer);
                        }
                    }

                    this.setWaterLevel(worldIn, pos, state, i - 1);
                }

                return true;
            }
            else
            {
                if (i > 0 && item instanceof ItemArmor)
                {
                    ItemArmor itemarmor = (ItemArmor)item;

                    if (itemarmor.getArmorMaterial() == ItemArmor.ArmorMaterial.LEATHER && itemarmor.hasColor(hitX) && !worldIn.isRemote)
                    {
                        itemarmor.removeColor(hitX);
                        this.setWaterLevel(worldIn, pos, state, i - 1);
                        playerIn.triggerAchievement(StatList.field_188079_M);
                        return true;
                    }
                }

                if (i > 0 && item instanceof ItemBanner)
                {
                    if (TileEntityBanner.getPatterns(hitX) > 0 && !worldIn.isRemote)
                    {
                        ItemStack itemstack = hitX.copy();
                        itemstack.stackSize = 1;
                        TileEntityBanner.removeBannerData(itemstack);
                        playerIn.triggerAchievement(StatList.field_188080_N);

                        if (!playerIn.capabilities.isCreativeMode)
                        {
                            --hitX.stackSize;
                        }

                        if (hitX.stackSize == 0)
                        {
                            playerIn.setHeldItem(side, itemstack);
                        }
                        else if (!playerIn.inventory.addItemStackToInventory(itemstack))
                        {
                            playerIn.dropPlayerItemWithRandomChoice(itemstack, false);
                        }
                        else if (playerIn instanceof EntityPlayerMP)
                        {
                            ((EntityPlayerMP)playerIn).sendContainerToPlayer(playerIn.inventoryContainer);
                        }

                        if (!playerIn.capabilities.isCreativeMode)
                        {
                            this.setWaterLevel(worldIn, pos, state, i - 1);
                        }
                    }

                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    public void setWaterLevel(World worldIn, BlockPos pos, IBlockState state, int level)
    {
        worldIn.setBlockState(pos, state.withProperty(LEVEL, Integer.valueOf(MathHelper.clamp_int(level, 0, 3))), 2);
        worldIn.updateComparatorOutputLevel(pos, this);
    }

    /**
     * Called similar to random ticks, but only when it is raining.
     */
    public void fillWithRain(World worldIn, BlockPos pos)
    {
        if (worldIn.rand.nextInt(20) == 1)
        {
            float f = worldIn.getBiomeGenForCoords(pos).getFloatTemperature(pos);

            if (worldIn.getWorldChunkManager().getTemperatureAtHeight(f, pos.getY()) >= 0.15F)
            {
                IBlockState iblockstate = worldIn.getBlockState(pos);

                if (((Integer)iblockstate.getValue(LEVEL)).intValue() < 3)
                {
                    worldIn.setBlockState(pos, iblockstate.cycleProperty(LEVEL), 2);
                }
            }
        }
    }

    /**
     * Get the Item that this Block should drop when harvested.
     */
    public Item getItemDropped(IBlockState state, Random rand, int fortune)
    {
        return Items.cauldron;
    }

    public ItemStack func_185473_a(World worldIn, BlockPos pos, IBlockState state)
    {
        return new ItemStack(Items.cauldron);
    }

    public boolean hasComparatorInputOverride(IBlockState state)
    {
        return true;
    }

    public int getComparatorInputOverride(IBlockState worldIn, World pos, BlockPos p_180641_3_)
    {
        return ((Integer)worldIn.getValue(LEVEL)).intValue();
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(LEVEL, Integer.valueOf(meta));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Integer)state.getValue(LEVEL)).intValue();
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {LEVEL});
    }

    public boolean isPassable(IBlockAccess worldIn, BlockPos pos)
    {
        return true;
    }
}
