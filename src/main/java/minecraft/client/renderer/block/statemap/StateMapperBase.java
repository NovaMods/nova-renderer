package net.minecraft.client.renderer.block.statemap;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Map.Entry;
import net.minecraft.block.Block;
import net.minecraft.block.properties.IProperty;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.block.model.ModelResourceLocation;

public abstract class StateMapperBase implements IStateMapper
{
    protected Map<IBlockState, ModelResourceLocation> mapStateModelLocations = Maps.<IBlockState, ModelResourceLocation>newLinkedHashMap();

    public String getPropertyString(Map < IProperty<?>, Comparable<? >> p_178131_1_)
    {
        StringBuilder stringbuilder = new StringBuilder();

        for (Entry < IProperty<?>, Comparable<? >> entry : p_178131_1_.entrySet())
        {
            if (stringbuilder.length() != 0)
            {
                stringbuilder.append(",");
            }

            IProperty<?> iproperty = (IProperty)entry.getKey();
            stringbuilder.append(iproperty.getName());
            stringbuilder.append("=");
            stringbuilder.append(this.func_187489_a(iproperty, (Comparable)entry.getValue()));
        }

        if (stringbuilder.length() == 0)
        {
            stringbuilder.append("normal");
        }

        return stringbuilder.toString();
    }

    private <T extends Comparable<T>> String func_187489_a(IProperty<T> p_187489_1_, Comparable<?> p_187489_2_)
    {
        return p_187489_1_.getName((T)p_187489_2_);
    }

    public Map<IBlockState, ModelResourceLocation> putStateModelLocations(Block blockIn)
    {
        for (IBlockState iblockstate : blockIn.getBlockState().getValidStates())
        {
            this.mapStateModelLocations.put(iblockstate, this.getModelResourceLocation(iblockstate));
        }

        return this.mapStateModelLocations;
    }

    protected abstract ModelResourceLocation getModelResourceLocation(IBlockState state);
}
