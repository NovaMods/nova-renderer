package net.minecraft.block.state.pattern;

import com.google.common.base.Predicate;
import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Map.Entry;
import net.minecraft.block.Block;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;

public class BlockStateMatcher implements Predicate<IBlockState>
{
    public static final Predicate<IBlockState> field_185928_a = new Predicate<IBlockState>()
    {
        public boolean apply(IBlockState p_apply_1_)
        {
            return true;
        }
    };
    private final BlockStateContainer blockstate;
    private final Map < IProperty<?>, Predicate<? >> propertyPredicates = Maps. < IProperty<?>, Predicate<? >> newHashMap();

    private BlockStateMatcher(BlockStateContainer blockStateIn)
    {
        this.blockstate = blockStateIn;
    }

    public static BlockStateMatcher forBlock(Block blockIn)
    {
        return new BlockStateMatcher(blockIn.getBlockState());
    }

    public boolean apply(IBlockState p_apply_1_)
    {
        if (p_apply_1_ != null && p_apply_1_.getBlock().equals(this.blockstate.getBlock()))
        {
            for (Entry < IProperty<?>, Predicate<? >> entry : this.propertyPredicates.entrySet())
            {
                if (!this.func_185927_a(p_apply_1_, (IProperty)entry.getKey(), (Predicate)entry.getValue()))
                {
                    return false;
                }
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    protected <T extends Comparable<T>> boolean func_185927_a(IBlockState p_185927_1_, IProperty<T> p_185927_2_, Predicate<T> p_185927_3_)
    {
        return p_185927_3_.apply(p_185927_1_.getValue(p_185927_2_));
    }

    public <V extends Comparable<V>> BlockStateMatcher where(IProperty<V> property, Predicate <? extends V > is)
    {
        if (!this.blockstate.getProperties().contains(property))
        {
            throw new IllegalArgumentException(this.blockstate + " cannot support property " + property);
        }
        else
        {
            this.propertyPredicates.put(property, is);
            return this;
        }
    }
}
