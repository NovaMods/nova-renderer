package net.minecraft.client.renderer.block.model.multipart;

import com.google.common.base.Function;
import com.google.common.base.Objects;
import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import com.google.common.base.Predicates;
import com.google.common.base.Splitter;
import com.google.common.collect.Iterables;
import java.util.List;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.BlockStateContainer;
import net.minecraft.block.state.IBlockState;

public class ConditionPropertyValue implements ICondition
{
    private static final Splitter field_188124_c = Splitter.on('|').omitEmptyStrings();
    private final String key;
    private final String value;

    public ConditionPropertyValue(String p_i46565_1_, String p_i46565_2_)
    {
        this.key = p_i46565_1_;
        this.value = p_i46565_2_;
    }

    public Predicate<IBlockState> func_188118_a(BlockStateContainer p_188118_1_)
    {
        final IProperty<?> iproperty = p_188118_1_.func_185920_a(this.key);

        if (iproperty == null)
        {
            throw new RuntimeException(this.toString() + ": Definition: " + p_188118_1_ + " has no property: " + this.key);
        }
        else
        {
            String s = this.value;
            boolean flag = !s.isEmpty() && s.charAt(0) == 33;

            if (flag)
            {
                s = s.substring(1);
            }

            List<String> list = field_188124_c.splitToList(s);

            if (list.isEmpty())
            {
                throw new RuntimeException(this.toString() + ": has an empty value: " + this.value);
            }
            else
            {
                Predicate<IBlockState> predicate;

                if (list.size() == 1)
                {
                    predicate = this.func_188123_a(iproperty, s);
                }
                else
                {
                    predicate = Predicates.or(Iterables.transform(list, new Function<String, Predicate<IBlockState>>()
                    {
                        public Predicate<IBlockState> apply(String p_apply_1_)
                        {
                            return ConditionPropertyValue.this.func_188123_a(iproperty, p_apply_1_);
                        }
                    }));
                }

                return flag ? Predicates.not(predicate) : predicate;
            }
        }
    }

    private Predicate<IBlockState> func_188123_a(final IProperty<?> p_188123_1_, String p_188123_2_)
    {
        final Optional<?> optional = p_188123_1_.func_185929_b(p_188123_2_);

        if (!optional.isPresent())
        {
            throw new RuntimeException(this.toString() + ": has an unknown value: " + this.value);
        }
        else
        {
            return new Predicate<IBlockState>()
            {
                public boolean apply(IBlockState p_apply_1_)
                {
                    return p_apply_1_ != null && p_apply_1_.getValue(p_188123_1_).equals(optional.get());
                }
            };
        }
    }

    public String toString()
    {
        return Objects.toStringHelper(this).add("key", this.key).add("value", this.value).toString();
    }
}
