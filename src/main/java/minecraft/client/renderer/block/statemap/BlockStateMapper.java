package net.minecraft.client.renderer.block.statemap;

import com.google.common.base.Objects;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.block.model.ModelResourceLocation;
import net.minecraft.util.ResourceLocation;

public class BlockStateMapper
{
    private Map<Block, IStateMapper> blockStateMap = Maps.<Block, IStateMapper>newIdentityHashMap();
    private Set<Block> setBuiltInBlocks = Sets.<Block>newIdentityHashSet();

    public void registerBlockStateMapper(Block p_178447_1_, IStateMapper p_178447_2_)
    {
        this.blockStateMap.put(p_178447_1_, p_178447_2_);
    }

    public void registerBuiltInBlocks(Block... p_178448_1_)
    {
        Collections.addAll(this.setBuiltInBlocks, p_178448_1_);
    }

    public Map<IBlockState, ModelResourceLocation> putAllStateModelLocations()
    {
        Map<IBlockState, ModelResourceLocation> map = Maps.<IBlockState, ModelResourceLocation>newIdentityHashMap();

        for (Block block : Block.blockRegistry)
        {
            map.putAll(this.getVariants(block));
        }

        return map;
    }

    public Set<ResourceLocation> getBlockstateLocations(Block p_188182_1_)
    {
        if (this.setBuiltInBlocks.contains(p_188182_1_))
        {
            return Collections.<ResourceLocation>emptySet();
        }
        else
        {
            IStateMapper istatemapper = (IStateMapper)this.blockStateMap.get(p_188182_1_);

            if (istatemapper == null)
            {
                return Collections.<ResourceLocation>singleton(Block.blockRegistry.getNameForObject(p_188182_1_));
            }
            else
            {
                Set<ResourceLocation> set = Sets.<ResourceLocation>newHashSet();

                for (ModelResourceLocation modelresourcelocation : istatemapper.putStateModelLocations(p_188182_1_).values())
                {
                    set.add(new ResourceLocation(modelresourcelocation.getResourceDomain(), modelresourcelocation.getResourcePath()));
                }

                return set;
            }
        }
    }

    public Map<IBlockState, ModelResourceLocation> getVariants(Block p_188181_1_)
    {
        return this.setBuiltInBlocks.contains(p_188181_1_) ? Collections.<IBlockState, ModelResourceLocation>emptyMap() : ((IStateMapper)Objects.firstNonNull(this.blockStateMap.get(p_188181_1_), new DefaultStateMapper())).putStateModelLocations(p_188181_1_);
    }
}
