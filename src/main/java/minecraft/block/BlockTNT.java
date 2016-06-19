package net.minecraft.block;

import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyBool;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.item.EntityTNTPrimed;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.item.ItemStack;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.Explosion;
import net.minecraft.world.World;

public class BlockTNT extends Block
{
    public static final PropertyBool EXPLODE = PropertyBool.create("explode");

    public BlockTNT()
    {
        super(Material.tnt);
        this.setDefaultState(this.blockState.getBaseState().withProperty(EXPLODE, Boolean.valueOf(false)));
        this.setCreativeTab(CreativeTabs.tabRedstone);
    }

    public void onBlockAdded(World worldIn, BlockPos pos, IBlockState state)
    {
        super.onBlockAdded(worldIn, pos, state);

        if (worldIn.isBlockPowered(pos))
        {
            this.onBlockDestroyedByPlayer(worldIn, pos, state.withProperty(EXPLODE, Boolean.valueOf(true)));
            worldIn.setBlockToAir(pos);
        }
    }

    /**
     * Called when a neighboring block changes.
     */
    public void onNeighborBlockChange(World worldIn, BlockPos pos, IBlockState state, Block neighborBlock)
    {
        if (worldIn.isBlockPowered(pos))
        {
            this.onBlockDestroyedByPlayer(worldIn, pos, state.withProperty(EXPLODE, Boolean.valueOf(true)));
            worldIn.setBlockToAir(pos);
        }
    }

    /**
     * Called when this Block is destroyed by an Explosion
     */
    public void onBlockDestroyedByExplosion(World worldIn, BlockPos pos, Explosion explosionIn)
    {
        if (!worldIn.isRemote)
        {
            EntityTNTPrimed entitytntprimed = new EntityTNTPrimed(worldIn, (double)((float)pos.getX() + 0.5F), (double)pos.getY(), (double)((float)pos.getZ() + 0.5F), explosionIn.getExplosivePlacedBy());
            entitytntprimed.func_184534_a((short)(worldIn.rand.nextInt(entitytntprimed.func_184536_l() / 4) + entitytntprimed.func_184536_l() / 8));
            worldIn.spawnEntityInWorld(entitytntprimed);
        }
    }

    /**
     * Called when a player destroys this Block
     */
    public void onBlockDestroyedByPlayer(World worldIn, BlockPos pos, IBlockState state)
    {
        this.explode(worldIn, pos, state, (EntityLivingBase)null);
    }

    public void explode(World worldIn, BlockPos pos, IBlockState state, EntityLivingBase igniter)
    {
        if (!worldIn.isRemote)
        {
            if (((Boolean)state.getValue(EXPLODE)).booleanValue())
            {
                EntityTNTPrimed entitytntprimed = new EntityTNTPrimed(worldIn, (double)((float)pos.getX() + 0.5F), (double)pos.getY(), (double)((float)pos.getZ() + 0.5F), igniter);
                worldIn.spawnEntityInWorld(entitytntprimed);
                worldIn.func_184148_a((EntityPlayer)null, entitytntprimed.posX, entitytntprimed.posY, entitytntprimed.posZ, SoundEvents.entity_tnt_primed, SoundCategory.BLOCKS, 1.0F, 1.0F);
            }
        }
    }

    public boolean onBlockActivated(World worldIn, BlockPos pos, IBlockState state, EntityPlayer playerIn, EnumHand side, ItemStack hitX, EnumFacing hitY, float hitZ, float p_180639_9_, float p_180639_10_)
    {
        if (hitX != null && (hitX.getItem() == Items.flint_and_steel || hitX.getItem() == Items.fire_charge))
        {
            this.explode(worldIn, pos, state.withProperty(EXPLODE, Boolean.valueOf(true)), playerIn);
            worldIn.setBlockState(pos, Blocks.air.getDefaultState(), 11);

            if (hitX.getItem() == Items.flint_and_steel)
            {
                hitX.damageItem(1, playerIn);
            }
            else if (!playerIn.capabilities.isCreativeMode)
            {
                --hitX.stackSize;
            }

            return true;
        }
        else
        {
            return super.onBlockActivated(worldIn, pos, state, playerIn, side, hitX, hitY, hitZ, p_180639_9_, p_180639_10_);
        }
    }

    /**
     * Called When an Entity Collided with the Block
     */
    public void onEntityCollidedWithBlock(World worldIn, BlockPos pos, IBlockState state, Entity entityIn)
    {
        if (!worldIn.isRemote && entityIn instanceof EntityArrow)
        {
            EntityArrow entityarrow = (EntityArrow)entityIn;

            if (entityarrow.isBurning())
            {
                this.explode(worldIn, pos, worldIn.getBlockState(pos).withProperty(EXPLODE, Boolean.valueOf(true)), entityarrow.shootingEntity instanceof EntityLivingBase ? (EntityLivingBase)entityarrow.shootingEntity : null);
                worldIn.setBlockToAir(pos);
            }
        }
    }

    /**
     * Return whether this block can drop from an explosion.
     */
    public boolean canDropFromExplosion(Explosion explosionIn)
    {
        return false;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(EXPLODE, Boolean.valueOf((meta & 1) > 0));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Boolean)state.getValue(EXPLODE)).booleanValue() ? 1 : 0;
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {EXPLODE});
    }
}
