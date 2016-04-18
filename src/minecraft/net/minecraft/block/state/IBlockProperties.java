package net.minecraft.block.state;

import java.util.List;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.block.material.MapColor;
import net.minecraft.block.material.Material;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.EnumBlockRenderType;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;

public interface IBlockProperties
{
    Material getMaterial();

    boolean isFullBlock();

    int getLightOpacity();

    int getlightValue();

    boolean isTranslucent();

    boolean useNeighborBrightness();

    MapColor getMapColor();

    /**
     * Returns the blockstate with the given rotation. If inapplicable, returns itself.
     */
    IBlockState withRotation(Rotation rot);

    /**
     * Returns the blockstate mirrored in the given way. If inapplicable, returns itself.
     */
    IBlockState withMirror(Mirror mirrorIn);

    boolean isFullCube();

    EnumBlockRenderType getRenderType();

    int getPackedLightmapCoords(IBlockAccess p_185889_1_, BlockPos p_185889_2_);

    float func_185892_j();

    boolean isBlockNormalCube();

    boolean isNormalCube();

    boolean canProvidePower();

    int getWeakPower(IBlockAccess p_185911_1_, BlockPos p_185911_2_, EnumFacing p_185911_3_);

    boolean hasComparatorInputOverride();

    int getComparatorInputOverride(World p_185888_1_, BlockPos p_185888_2_);

    float getBlockHardness(World p_185887_1_, BlockPos p_185887_2_);

    float getPlayerRelativeBlockHardness(EntityPlayer p_185903_1_, World p_185903_2_, BlockPos p_185903_3_);

    int getStrongPower(IBlockAccess p_185893_1_, BlockPos p_185893_2_, EnumFacing p_185893_3_);

    EnumPushReaction getMobilityFlag();

    IBlockState getActualState(IBlockAccess p_185899_1_, BlockPos p_185899_2_);

    AxisAlignedBB getCollisionBoundingBox(World p_185918_1_, BlockPos p_185918_2_);

    boolean shouldSideBeRendered(IBlockAccess p_185894_1_, BlockPos p_185894_2_, EnumFacing p_185894_3_);

    boolean isOpaqueCube();

    AxisAlignedBB getSelectedBoundingBox(World p_185890_1_, BlockPos p_185890_2_);

    void func_185908_a(World p_185908_1_, BlockPos p_185908_2_, AxisAlignedBB p_185908_3_, List<AxisAlignedBB> p_185908_4_, Entity p_185908_5_);

    AxisAlignedBB func_185900_c(IBlockAccess p_185900_1_, BlockPos p_185900_2_);

    RayTraceResult func_185910_a(World p_185910_1_, BlockPos p_185910_2_, Vec3d p_185910_3_, Vec3d p_185910_4_);

    boolean func_185896_q();
}
