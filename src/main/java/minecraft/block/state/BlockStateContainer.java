package net.minecraft.block.state;

import com.google.common.base.Function;
import com.google.common.base.Objects;
import com.google.common.collect.HashBasedTable;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSortedMap;
import com.google.common.collect.ImmutableTable;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Table;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Pattern;
import net.minecraft.block.Block;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.block.properties.IProperty;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.MapPopulator;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Cartesian;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public class BlockStateContainer
{
    private static final Pattern field_185921_a = Pattern.compile("^[a-z0-9_]+$");
    private static final Function < IProperty<?>, String > GET_NAME_FUNC = new Function < IProperty<?>, String > ()
    {
        public String apply(IProperty<?> p_apply_1_)
        {
            return p_apply_1_ == null ? "<NULL>" : p_apply_1_.getName();
        }
    };
    private final Block block;
    private final ImmutableSortedMap < String, IProperty<? >> properties;
    private final ImmutableList<IBlockState> validStates;

    public BlockStateContainer(Block blockIn, IProperty<?>... properties)
    {
        this.block = blockIn;
        Map < String, IProperty<? >> map = Maps. < String, IProperty<? >> newHashMap();

        for (IProperty<?> iproperty : properties)
        {
            func_185919_a(blockIn, iproperty);
            map.put(iproperty.getName(), iproperty);
        }

        this.properties = ImmutableSortedMap. < String, IProperty<? >> copyOf(map);
        Map < Map < IProperty<?>, Comparable<? >> , BlockStateContainer.StateImplementation > map2 = Maps. < Map < IProperty<?>, Comparable<? >> , BlockStateContainer.StateImplementation > newLinkedHashMap();
        List<BlockStateContainer.StateImplementation> list1 = Lists.<BlockStateContainer.StateImplementation>newArrayList();

        for (List < Comparable<? >> list : Cartesian.cartesianProduct(this.getAllowedValues()))
        {
            Map < IProperty<?>, Comparable<? >> map1 = MapPopulator. < IProperty<?>, Comparable<? >> createMap(this.properties.values(), list);
            BlockStateContainer.StateImplementation blockstatecontainer$stateimplementation = new BlockStateContainer.StateImplementation(blockIn, ImmutableMap.copyOf(map1));
            map2.put(map1, blockstatecontainer$stateimplementation);
            list1.add(blockstatecontainer$stateimplementation);
        }

        for (BlockStateContainer.StateImplementation blockstatecontainer$stateimplementation1 : list1)
        {
            blockstatecontainer$stateimplementation1.buildPropertyValueTable(map2);
        }

        this.validStates = ImmutableList.<IBlockState>copyOf(list1);
    }

    public static <T extends Comparable<T>> String func_185919_a(Block p_185919_0_, IProperty<T> p_185919_1_)
    {
        String s = p_185919_1_.getName();

        if (!field_185921_a.matcher(s).matches())
        {
            throw new IllegalArgumentException("Block: " + p_185919_0_.getClass() + " has invalidly named property: " + s);
        }
        else
        {
            for (T t : p_185919_1_.getAllowedValues())
            {
                String s1 = p_185919_1_.getName(t);

                if (!field_185921_a.matcher(s1).matches())
                {
                    throw new IllegalArgumentException("Block: " + p_185919_0_.getClass() + " has property: " + s + " with invalidly named value: " + s1);
                }
            }

            return s;
        }
    }

    public ImmutableList<IBlockState> getValidStates()
    {
        return this.validStates;
    }

    private List < Iterable < Comparable<? >>> getAllowedValues()
    {
        List < Iterable < Comparable<? >>> list = Lists. < Iterable < Comparable<? >>> newArrayList();

        for (IProperty<?> iproperty : this.properties.values())
        {
            list.add(((IProperty)iproperty).getAllowedValues());
        }

        return list;
    }

    public IBlockState getBaseState()
    {
        return (IBlockState)this.validStates.get(0);
    }

    public Block getBlock()
    {
        return this.block;
    }

    public Collection < IProperty<? >> getProperties()
    {
        return this.properties.values();
    }

    public String toString()
    {
        return Objects.toStringHelper(this).add("block", Block.blockRegistry.getNameForObject(this.block)).add("properties", Iterables.transform(this.properties.values(), GET_NAME_FUNC)).toString();
    }

    public IProperty<?> func_185920_a(String p_185920_1_)
    {
        return (IProperty)this.properties.get(p_185920_1_);
    }

    static class StateImplementation extends BlockStateBase
    {
        private final Block block;
        private final ImmutableMap < IProperty<?>, Comparable<? >> properties;
        private ImmutableTable < IProperty<?>, Comparable<?>, IBlockState > propertyValueTable;

        private StateImplementation(Block blockIn, ImmutableMap < IProperty<?>, Comparable<? >> propertiesIn)
        {
            this.block = blockIn;
            this.properties = propertiesIn;
        }

        public Collection < IProperty<? >> getPropertyNames()
        {
            return Collections. < IProperty<? >> unmodifiableCollection(this.properties.keySet());
        }

        public <T extends Comparable<T>> T getValue(IProperty<T> property)
        {
            if (!this.properties.containsKey(property))
            {
                throw new IllegalArgumentException("Cannot get property " + property + " as it does not exist in " + this.block.getBlockState());
            }
            else
            {
                return (T)((Comparable)property.getValueClass().cast(this.properties.get(property)));
            }
        }

        public <T extends Comparable<T>, V> IBlockState withProperty(IProperty<T> property, V value)
        {
            if (!this.properties.containsKey(property))
            {
                throw new IllegalArgumentException("Cannot set property " + property + " as it does not exist in " + this.block.getBlockState());
            }
            else if (!property.getAllowedValues().contains(value))
            {
                throw new IllegalArgumentException("Cannot set property " + property + " to " + value + " on block " + Block.blockRegistry.getNameForObject(this.block) + ", it is not an allowed value");
            }
            else
            {
                return (IBlockState)(this.properties.get(property) == value ? this : (IBlockState)this.propertyValueTable.get(property, value));
            }
        }

        public ImmutableMap < IProperty<?>, Comparable<? >> getProperties()
        {
            return this.properties;
        }

        public Block getBlock()
        {
            return this.block;
        }

        public boolean equals(Object p_equals_1_)
        {
            return this == p_equals_1_;
        }

        public int hashCode()
        {
            return this.properties.hashCode();
        }

        public void buildPropertyValueTable(Map < Map < IProperty<?>, Comparable<? >> , BlockStateContainer.StateImplementation > map)
        {
            if (this.propertyValueTable != null)
            {
                throw new IllegalStateException();
            }
            else
            {
                Table < IProperty<?>, Comparable<?>, IBlockState > table = HashBasedTable. < IProperty<?>, Comparable<?>, IBlockState > create();

                for (Entry < IProperty<?>, Comparable<? >> entry : this.properties.entrySet())
                {
                    IProperty<?> iproperty = (IProperty)entry.getKey();

                    for (Comparable<?> comparable : iproperty.getAllowedValues())
                    {
                        if (comparable != entry.getValue())
                        {
                            table.put(iproperty, comparable, map.get(this.getPropertiesWithValue(iproperty, comparable)));
                        }
                    }
                }

                this.propertyValueTable = ImmutableTable. < IProperty<?>, Comparable<?>, IBlockState > copyOf(table);
            }
        }

        private Map < IProperty<?>, Comparable<? >> getPropertiesWithValue(IProperty<?> property, Comparable<?> value)
        {
            Map < IProperty<?>, Comparable<? >> map = Maps. < IProperty<?>, Comparable<? >> newHashMap(this.properties);
            map.put(property, value);
            return map;
        }

        public Material getMaterial()
        {
            return this.block.getMaterial(this);
        }

        public boolean isFullBlock()
        {
            return this.block.isFullBlock(this);
        }

        public int getLightOpacity()
        {
            return this.block.getLightOpacity(this);
        }

        public int getlightValue()
        {
            return this.block.getLightValue(this);
        }

        public boolean isTranslucent()
        {
            return this.block.isTranslucent(this);
        }

        public boolean useNeighborBrightness()
        {
            return this.block.getUseNeighborBrightness(this);
        }

        public MapColor getMapColor()
        {
            return this.block.getMapColor(this);
        }

        public IBlockState withRotation(Rotation rot)
        {
            return this.block.withRotation(this, rot);
        }

        public IBlockState withMirror(Mirror mirrorIn)
        {
            return this.block.withMirror(this, mirrorIn);
        }

        public boolean isFullCube()
        {
            return this.block.isFullCube(this);
        }

        public EnumBlockRenderType getRenderType()
        {
            return this.block.getRenderType(this);
        }

        public int getPackedLightmapCoords(IBlockAccess p_185889_1_, BlockPos p_185889_2_)
        {
            return this.block.getPackedLightmapCoords(this, p_185889_1_, p_185889_2_);
        }

        public float func_185892_j()
        {
            return this.block.func_185485_f(this);
        }

        public boolean isBlockNormalCube()
        {
            return this.block.isBlockNormalCube(this);
        }

        public boolean isNormalCube()
        {
            return this.block.isNormalCube(this);
        }

        public boolean canProvidePower()
        {
            return this.block.canProvidePower(this);
        }

        public int getWeakPower(IBlockAccess p_185911_1_, BlockPos p_185911_2_, EnumFacing p_185911_3_)
        {
            return this.block.getWeakPower(this, p_185911_1_, p_185911_2_, p_185911_3_);
        }

        public boolean hasComparatorInputOverride()
        {
            return this.block.hasComparatorInputOverride(this);
        }

        public int getComparatorInputOverride(World p_185888_1_, BlockPos p_185888_2_)
        {
            return this.block.getComparatorInputOverride(this, p_185888_1_, p_185888_2_);
        }

        public float getBlockHardness(World p_185887_1_, BlockPos p_185887_2_)
        {
            return this.block.getBlockHardness(this, p_185887_1_, p_185887_2_);
        }

        public float getPlayerRelativeBlockHardness(EntityPlayer p_185903_1_, World p_185903_2_, BlockPos p_185903_3_)
        {
            return this.block.getPlayerRelativeBlockHardness(this, p_185903_1_, p_185903_2_, p_185903_3_);
        }

        public int getStrongPower(IBlockAccess p_185893_1_, BlockPos p_185893_2_, EnumFacing p_185893_3_)
        {
            return this.block.getStrongPower(this, p_185893_1_, p_185893_2_, p_185893_3_);
        }

        public EnumPushReaction getMobilityFlag()
        {
            return this.block.getMobilityFlag(this);
        }

        public IBlockState getActualState(IBlockAccess p_185899_1_, BlockPos p_185899_2_)
        {
            return this.block.getActualState(this, p_185899_1_, p_185899_2_);
        }

        public AxisAlignedBB getCollisionBoundingBox(World p_185918_1_, BlockPos p_185918_2_)
        {
            return this.block.getCollisionBoundingBox(this, p_185918_1_, p_185918_2_);
        }

        public boolean shouldSideBeRendered(IBlockAccess p_185894_1_, BlockPos p_185894_2_, EnumFacing p_185894_3_)
        {
            return this.block.shouldSideBeRendered(this, p_185894_1_, p_185894_2_, p_185894_3_);
        }

        public boolean isOpaqueCube()
        {
            return this.block.isOpaqueCube(this);
        }

        public AxisAlignedBB getSelectedBoundingBox(World p_185890_1_, BlockPos p_185890_2_)
        {
            return this.block.getSelectedBoundingBox(this, p_185890_1_, p_185890_2_);
        }

        public void func_185908_a(World p_185908_1_, BlockPos p_185908_2_, AxisAlignedBB p_185908_3_, List<AxisAlignedBB> p_185908_4_, Entity p_185908_5_)
        {
            this.block.func_185477_a(this, p_185908_1_, p_185908_2_, p_185908_3_, p_185908_4_, p_185908_5_);
        }

        public AxisAlignedBB func_185900_c(IBlockAccess p_185900_1_, BlockPos p_185900_2_)
        {
            return this.block.func_185496_a(this, p_185900_1_, p_185900_2_);
        }

        public RayTraceResult func_185910_a(World p_185910_1_, BlockPos p_185910_2_, Vec3d p_185910_3_, Vec3d p_185910_4_)
        {
            return this.block.collisionRayTrace(this, p_185910_1_, p_185910_2_, p_185910_3_, p_185910_4_);
        }

        public boolean func_185896_q()
        {
            return this.block.isFullyOpaque(this);
        }
    }
}
