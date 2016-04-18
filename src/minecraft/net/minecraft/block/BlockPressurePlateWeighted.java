package net.minecraft.block;

import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.properties.PropertyInteger;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.SoundEvents;
import net.minecraft.util.SoundCategory;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class BlockPressurePlateWeighted extends BlockBasePressurePlate
{
    public static final PropertyInteger POWER = PropertyInteger.create("power", 0, 15);
    private final int field_150068_a;

    protected BlockPressurePlateWeighted(Material p_i46379_1_, int p_i46379_2_)
    {
        this(p_i46379_1_, p_i46379_2_, p_i46379_1_.getMaterialMapColor());
    }

    protected BlockPressurePlateWeighted(Material p_i46380_1_, int p_i46380_2_, MapColor p_i46380_3_)
    {
        super(p_i46380_1_, p_i46380_3_);
        this.setDefaultState(this.blockState.getBaseState().withProperty(POWER, Integer.valueOf(0)));
        this.field_150068_a = p_i46380_2_;
    }

    protected int computeRedstoneStrength(World worldIn, BlockPos pos)
    {
        int i = Math.min(worldIn.getEntitiesWithinAABB(Entity.class, field_185511_c.offset(pos)).size(), this.field_150068_a);

        if (i > 0)
        {
            float f = (float)Math.min(this.field_150068_a, i) / (float)this.field_150068_a;
            return MathHelper.ceiling_float_int(f * 15.0F);
        }
        else
        {
            return 0;
        }
    }

    protected void func_185507_b(World p_185507_1_, BlockPos p_185507_2_)
    {
        p_185507_1_.func_184133_a((EntityPlayer)null, p_185507_2_, SoundEvents.block_metal_pressplate_click_on, SoundCategory.BLOCKS, 0.3F, 0.90000004F);
    }

    protected void func_185508_c(World p_185508_1_, BlockPos p_185508_2_)
    {
        p_185508_1_.func_184133_a((EntityPlayer)null, p_185508_2_, SoundEvents.block_metal_pressplate_click_off, SoundCategory.BLOCKS, 0.3F, 0.75F);
    }

    protected int getRedstoneStrength(IBlockState state)
    {
        return ((Integer)state.getValue(POWER)).intValue();
    }

    protected IBlockState setRedstoneStrength(IBlockState state, int strength)
    {
        return state.withProperty(POWER, Integer.valueOf(strength));
    }

    /**
     * How many world ticks before ticking
     */
    public int tickRate(World worldIn)
    {
        return 10;
    }

    /**
     * Convert the given metadata into a BlockState for this Block
     */
    public IBlockState getStateFromMeta(int meta)
    {
        return this.getDefaultState().withProperty(POWER, Integer.valueOf(meta));
    }

    /**
     * Convert the BlockState into the correct metadata value
     */
    public int getMetaFromState(IBlockState state)
    {
        return ((Integer)state.getValue(POWER)).intValue();
    }

    protected BlockStateContainer createBlockState()
    {
        return new BlockStateContainer(this, new IProperty[] {POWER});
    }
}
