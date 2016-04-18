package net.minecraft.world.gen.structure;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.Random;
import net.minecraft.block.BlockRail;
import net.minecraft.block.BlockRailBase;
import net.minecraft.block.BlockTorch;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.item.EntityMinecartChest;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityMobSpawner;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

@SuppressWarnings("incomplete-switch")
public class StructureMineshaftPieces
{
    public static void registerStructurePieces()
    {
        MapGenStructureIO.registerStructureComponent(StructureMineshaftPieces.Corridor.class, "MSCorridor");
        MapGenStructureIO.registerStructureComponent(StructureMineshaftPieces.Cross.class, "MSCrossing");
        MapGenStructureIO.registerStructureComponent(StructureMineshaftPieces.Room.class, "MSRoom");
        MapGenStructureIO.registerStructureComponent(StructureMineshaftPieces.Stairs.class, "MSStairs");
    }

    private static StructureComponent func_175892_a(List<StructureComponent> listIn, Random rand, int x, int y, int z, EnumFacing facing, int type)
    {
        int i = rand.nextInt(100);

        if (i >= 80)
        {
            StructureBoundingBox structureboundingbox = StructureMineshaftPieces.Cross.func_175813_a(listIn, rand, x, y, z, facing);

            if (structureboundingbox != null)
            {
                return new StructureMineshaftPieces.Cross(type, rand, structureboundingbox, facing);
            }
        }
        else if (i >= 70)
        {
            StructureBoundingBox structureboundingbox1 = StructureMineshaftPieces.Stairs.func_175812_a(listIn, rand, x, y, z, facing);

            if (structureboundingbox1 != null)
            {
                return new StructureMineshaftPieces.Stairs(type, rand, structureboundingbox1, facing);
            }
        }
        else
        {
            StructureBoundingBox structureboundingbox2 = StructureMineshaftPieces.Corridor.func_175814_a(listIn, rand, x, y, z, facing);

            if (structureboundingbox2 != null)
            {
                return new StructureMineshaftPieces.Corridor(type, rand, structureboundingbox2, facing);
            }
        }

        return null;
    }

    private static StructureComponent func_175890_b(StructureComponent componentIn, List<StructureComponent> listIn, Random rand, int x, int y, int z, EnumFacing facing, int type)
    {
        if (type > 8)
        {
            return null;
        }
        else if (Math.abs(x - componentIn.getBoundingBox().minX) <= 80 && Math.abs(z - componentIn.getBoundingBox().minZ) <= 80)
        {
            StructureComponent structurecomponent = func_175892_a(listIn, rand, x, y, z, facing, type + 1);

            if (structurecomponent != null)
            {
                listIn.add(structurecomponent);
                structurecomponent.buildComponent(componentIn, listIn, rand);
            }

            return structurecomponent;
        }
        else
        {
            return null;
        }
    }

    public static class Corridor extends StructureComponent
    {
        private boolean hasRails;
        private boolean hasSpiders;
        private boolean spawnerPlaced;
        private int sectionCount;

        public Corridor()
        {
        }

        protected void writeStructureToNBT(NBTTagCompound tagCompound)
        {
            tagCompound.setBoolean("hr", this.hasRails);
            tagCompound.setBoolean("sc", this.hasSpiders);
            tagCompound.setBoolean("hps", this.spawnerPlaced);
            tagCompound.setInteger("Num", this.sectionCount);
        }

        protected void readStructureFromNBT(NBTTagCompound tagCompound)
        {
            this.hasRails = tagCompound.getBoolean("hr");
            this.hasSpiders = tagCompound.getBoolean("sc");
            this.spawnerPlaced = tagCompound.getBoolean("hps");
            this.sectionCount = tagCompound.getInteger("Num");
        }

        public Corridor(int type, Random rand, StructureBoundingBox structurebb, EnumFacing facing)
        {
            super(type);
            this.func_186164_a(facing);
            this.boundingBox = structurebb;
            this.hasRails = rand.nextInt(3) == 0;
            this.hasSpiders = !this.hasRails && rand.nextInt(23) == 0;

            if (this.func_186165_e().getAxis() == EnumFacing.Axis.Z)
            {
                this.sectionCount = structurebb.getZSize() / 5;
            }
            else
            {
                this.sectionCount = structurebb.getXSize() / 5;
            }
        }

        public static StructureBoundingBox func_175814_a(List<StructureComponent> p_175814_0_, Random rand, int x, int y, int z, EnumFacing facing)
        {
            StructureBoundingBox structureboundingbox = new StructureBoundingBox(x, y, z, x, y + 2, z);
            int i;

            for (i = rand.nextInt(3) + 2; i > 0; --i)
            {
                int j = i * 5;

                switch (facing)
                {
                    case NORTH:
                        structureboundingbox.maxX = x + 2;
                        structureboundingbox.minZ = z - (j - 1);
                        break;

                    case SOUTH:
                        structureboundingbox.maxX = x + 2;
                        structureboundingbox.maxZ = z + (j - 1);
                        break;

                    case WEST:
                        structureboundingbox.minX = x - (j - 1);
                        structureboundingbox.maxZ = z + 2;
                        break;

                    case EAST:
                        structureboundingbox.maxX = x + (j - 1);
                        structureboundingbox.maxZ = z + 2;
                }

                if (StructureComponent.findIntersecting(p_175814_0_, structureboundingbox) == null)
                {
                    break;
                }
            }

            return i > 0 ? structureboundingbox : null;
        }

        public void buildComponent(StructureComponent componentIn, List<StructureComponent> listIn, Random rand)
        {
            int i = this.getComponentType();
            int j = rand.nextInt(4);
            EnumFacing enumfacing = this.func_186165_e();

            if (enumfacing != null)
            {
                switch (enumfacing)
                {
                    case NORTH:
                        if (j <= 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ - 1, enumfacing, i);
                        }
                        else if (j == 2)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ, EnumFacing.WEST, i);
                        }
                        else
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ, EnumFacing.EAST, i);
                        }

                        break;

                    case SOUTH:
                        if (j <= 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.maxZ + 1, enumfacing, i);
                        }
                        else if (j == 2)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.maxZ - 3, EnumFacing.WEST, i);
                        }
                        else
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.maxZ - 3, EnumFacing.EAST, i);
                        }

                        break;

                    case WEST:
                        if (j <= 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ, enumfacing, i);
                        }
                        else if (j == 2)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                        }
                        else
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                        }

                        break;

                    case EAST:
                        if (j <= 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ, enumfacing, i);
                        }
                        else if (j == 2)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX - 3, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                        }
                        else
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX - 3, this.boundingBox.minY - 1 + rand.nextInt(3), this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                        }
                }
            }

            if (i < 8)
            {
                if (enumfacing != EnumFacing.NORTH && enumfacing != EnumFacing.SOUTH)
                {
                    for (int i1 = this.boundingBox.minX + 3; i1 + 3 <= this.boundingBox.maxX; i1 += 5)
                    {
                        int j1 = rand.nextInt(5);

                        if (j1 == 0)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, i1, this.boundingBox.minY, this.boundingBox.minZ - 1, EnumFacing.NORTH, i + 1);
                        }
                        else if (j1 == 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, i1, this.boundingBox.minY, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i + 1);
                        }
                    }
                }
                else
                {
                    for (int k = this.boundingBox.minZ + 3; k + 3 <= this.boundingBox.maxZ; k += 5)
                    {
                        int l = rand.nextInt(5);

                        if (l == 0)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY, k, EnumFacing.WEST, i + 1);
                        }
                        else if (l == 1)
                        {
                            StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY, k, EnumFacing.EAST, i + 1);
                        }
                    }
                }
            }
        }

        protected boolean func_186167_a(World p_186167_1_, StructureBoundingBox p_186167_2_, Random p_186167_3_, int p_186167_4_, int p_186167_5_, int p_186167_6_, ResourceLocation p_186167_7_)
        {
            BlockPos blockpos = new BlockPos(this.getXWithOffset(p_186167_4_, p_186167_6_), this.getYWithOffset(p_186167_5_), this.getZWithOffset(p_186167_4_, p_186167_6_));

            if (p_186167_2_.isVecInside(blockpos) && p_186167_1_.getBlockState(blockpos).getMaterial() == Material.air)
            {
                IBlockState iblockstate = Blocks.rail.getDefaultState().withProperty(BlockRail.SHAPE, p_186167_3_.nextBoolean() ? BlockRailBase.EnumRailDirection.NORTH_SOUTH : BlockRailBase.EnumRailDirection.EAST_WEST);
                this.setBlockState(p_186167_1_, iblockstate, p_186167_4_, p_186167_5_, p_186167_6_, p_186167_2_);
                EntityMinecartChest entityminecartchest = new EntityMinecartChest(p_186167_1_, (double)((float)blockpos.getX() + 0.5F), (double)((float)blockpos.getY() + 0.5F), (double)((float)blockpos.getZ() + 0.5F));
                entityminecartchest.func_184289_a(p_186167_7_, p_186167_3_.nextLong());
                p_186167_1_.spawnEntityInWorld(entityminecartchest);
                return true;
            }
            else
            {
                return false;
            }
        }

        public boolean addComponentParts(World worldIn, Random randomIn, StructureBoundingBox structureBoundingBoxIn)
        {
            if (this.isLiquidInStructureBoundingBox(worldIn, structureBoundingBoxIn))
            {
                return false;
            }
            else
            {
                int i = 0;
                int j = 2;
                int k = 0;
                int l = 2;
                int i1 = this.sectionCount * 5 - 1;
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 0, 0, 2, 1, i1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                this.func_175805_a(worldIn, structureBoundingBoxIn, randomIn, 0.8F, 0, 2, 0, 2, 2, i1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);

                if (this.hasSpiders)
                {
                    this.func_175805_a(worldIn, structureBoundingBoxIn, randomIn, 0.6F, 0, 0, 0, 2, 1, i1, Blocks.web.getDefaultState(), Blocks.air.getDefaultState(), false);
                }

                for (int j1 = 0; j1 < this.sectionCount; ++j1)
                {
                    int k1 = 2 + j1 * 5;
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 0, k1, 0, 1, k1, Blocks.oak_fence.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, 2, 0, k1, 2, 1, k1, Blocks.oak_fence.getDefaultState(), Blocks.air.getDefaultState(), false);

                    if (randomIn.nextInt(4) == 0)
                    {
                        this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 2, k1, 0, 2, k1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                        this.fillWithBlocks(worldIn, structureBoundingBoxIn, 2, 2, k1, 2, 2, k1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                    }
                    else
                    {
                        this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 2, k1, 2, 2, k1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                    }

                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.1F, 0, 2, k1 - 1, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.1F, 2, 2, k1 - 1, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.1F, 0, 2, k1 + 1, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.1F, 2, 2, k1 + 1, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 0, 2, k1 - 2, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 2, 2, k1 - 2, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 0, 2, k1 + 2, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 2, 2, k1 + 2, Blocks.web.getDefaultState());
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 1, 2, k1 - 1, Blocks.torch.getDefaultState().withProperty(BlockTorch.FACING, EnumFacing.SOUTH));
                    this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.05F, 1, 2, k1 + 1, Blocks.torch.getDefaultState().withProperty(BlockTorch.FACING, EnumFacing.NORTH));

                    if (randomIn.nextInt(100) == 0)
                    {
                        this.func_186167_a(worldIn, structureBoundingBoxIn, randomIn, 2, 0, k1 - 1, LootTableList.CHESTS_ABANDONED_MINESHAFT);
                    }

                    if (randomIn.nextInt(100) == 0)
                    {
                        this.func_186167_a(worldIn, structureBoundingBoxIn, randomIn, 0, 0, k1 + 1, LootTableList.CHESTS_ABANDONED_MINESHAFT);
                    }

                    if (this.hasSpiders && !this.spawnerPlaced)
                    {
                        int l1 = this.getYWithOffset(0);
                        int i2 = k1 - 1 + randomIn.nextInt(3);
                        int j2 = this.getXWithOffset(1, i2);
                        i2 = this.getZWithOffset(1, i2);
                        BlockPos blockpos = new BlockPos(j2, l1, i2);

                        if (structureBoundingBoxIn.isVecInside(blockpos))
                        {
                            this.spawnerPlaced = true;
                            worldIn.setBlockState(blockpos, Blocks.mob_spawner.getDefaultState(), 2);
                            TileEntity tileentity = worldIn.getTileEntity(blockpos);

                            if (tileentity instanceof TileEntityMobSpawner)
                            {
                                ((TileEntityMobSpawner)tileentity).getSpawnerBaseLogic().setEntityName("CaveSpider");
                            }
                        }
                    }
                }

                for (int k2 = 0; k2 <= 2; ++k2)
                {
                    for (int l2 = 0; l2 <= i1; ++l2)
                    {
                        int j3 = -1;
                        IBlockState iblockstate2 = this.getBlockStateFromPos(worldIn, k2, j3, l2, structureBoundingBoxIn);

                        if (iblockstate2.getMaterial() == Material.air)
                        {
                            int k3 = -1;
                            this.setBlockState(worldIn, Blocks.planks.getDefaultState(), k2, k3, l2, structureBoundingBoxIn);
                        }
                    }
                }

                if (this.hasRails)
                {
                    IBlockState iblockstate = Blocks.rail.getDefaultState().withProperty(BlockRail.SHAPE, BlockRailBase.EnumRailDirection.NORTH_SOUTH);

                    for (int i3 = 0; i3 <= i1; ++i3)
                    {
                        IBlockState iblockstate1 = this.getBlockStateFromPos(worldIn, 1, -1, i3, structureBoundingBoxIn);

                        if (iblockstate1.getMaterial() != Material.air && iblockstate1.isFullBlock())
                        {
                            this.randomlyPlaceBlock(worldIn, structureBoundingBoxIn, randomIn, 0.7F, 1, 0, i3, iblockstate);
                        }
                    }
                }

                return true;
            }
        }
    }

    public static class Cross extends StructureComponent
    {
        private EnumFacing corridorDirection;
        private boolean isMultipleFloors;

        public Cross()
        {
        }

        protected void writeStructureToNBT(NBTTagCompound tagCompound)
        {
            tagCompound.setBoolean("tf", this.isMultipleFloors);
            tagCompound.setInteger("D", this.corridorDirection.getHorizontalIndex());
        }

        protected void readStructureFromNBT(NBTTagCompound tagCompound)
        {
            this.isMultipleFloors = tagCompound.getBoolean("tf");
            this.corridorDirection = EnumFacing.getHorizontal(tagCompound.getInteger("D"));
        }

        public Cross(int type, Random rand, StructureBoundingBox structurebb, EnumFacing facing)
        {
            super(type);
            this.corridorDirection = facing;
            this.boundingBox = structurebb;
            this.isMultipleFloors = structurebb.getYSize() > 3;
        }

        public static StructureBoundingBox func_175813_a(List<StructureComponent> listIn, Random rand, int x, int y, int z, EnumFacing facing)
        {
            StructureBoundingBox structureboundingbox = new StructureBoundingBox(x, y, z, x, y + 2, z);

            if (rand.nextInt(4) == 0)
            {
                structureboundingbox.maxY += 4;
            }

            switch (facing)
            {
                case NORTH:
                    structureboundingbox.minX = x - 1;
                    structureboundingbox.maxX = x + 3;
                    structureboundingbox.minZ = z - 4;
                    break;

                case SOUTH:
                    structureboundingbox.minX = x - 1;
                    structureboundingbox.maxX = x + 3;
                    structureboundingbox.maxZ = z + 3 + 1;
                    break;

                case WEST:
                    structureboundingbox.minX = x - 4;
                    structureboundingbox.minZ = z - 1;
                    structureboundingbox.maxZ = z + 3;
                    break;

                case EAST:
                    structureboundingbox.maxX = x + 3 + 1;
                    structureboundingbox.minZ = z - 1;
                    structureboundingbox.maxZ = z + 3;
            }

            return StructureComponent.findIntersecting(listIn, structureboundingbox) != null ? null : structureboundingbox;
        }

        public void buildComponent(StructureComponent componentIn, List<StructureComponent> listIn, Random rand)
        {
            int i = this.getComponentType();

            switch (this.corridorDirection)
            {
                case NORTH:
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.WEST, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.EAST, i);
                    break;

                case SOUTH:
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.WEST, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.EAST, i);
                    break;

                case WEST:
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.WEST, i);
                    break;

                case EAST:
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY, this.boundingBox.minZ + 1, EnumFacing.EAST, i);
            }

            if (this.isMultipleFloors)
            {
                if (rand.nextBoolean())
                {
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY + 3 + 1, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                }

                if (rand.nextBoolean())
                {
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY + 3 + 1, this.boundingBox.minZ + 1, EnumFacing.WEST, i);
                }

                if (rand.nextBoolean())
                {
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY + 3 + 1, this.boundingBox.minZ + 1, EnumFacing.EAST, i);
                }

                if (rand.nextBoolean())
                {
                    StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + 1, this.boundingBox.minY + 3 + 1, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                }
            }
        }

        public boolean addComponentParts(World worldIn, Random randomIn, StructureBoundingBox structureBoundingBoxIn)
        {
            if (this.isLiquidInStructureBoundingBox(worldIn, structureBoundingBoxIn))
            {
                return false;
            }
            else
            {
                if (this.isMultipleFloors)
                {
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ, this.boundingBox.maxX - 1, this.boundingBox.minY + 3 - 1, this.boundingBox.maxZ, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.minY, this.boundingBox.minZ + 1, this.boundingBox.maxX, this.boundingBox.minY + 3 - 1, this.boundingBox.maxZ - 1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.maxY - 2, this.boundingBox.minZ, this.boundingBox.maxX - 1, this.boundingBox.maxY, this.boundingBox.maxZ, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.maxY - 2, this.boundingBox.minZ + 1, this.boundingBox.maxX, this.boundingBox.maxY, this.boundingBox.maxZ - 1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.minY + 3, this.boundingBox.minZ + 1, this.boundingBox.maxX - 1, this.boundingBox.minY + 3, this.boundingBox.maxZ - 1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                }
                else
                {
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ, this.boundingBox.maxX - 1, this.boundingBox.maxY, this.boundingBox.maxZ, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.minY, this.boundingBox.minZ + 1, this.boundingBox.maxX, this.boundingBox.maxY, this.boundingBox.maxZ - 1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                }

                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.minZ + 1, this.boundingBox.minX + 1, this.boundingBox.maxY, this.boundingBox.minZ + 1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX + 1, this.boundingBox.minY, this.boundingBox.maxZ - 1, this.boundingBox.minX + 1, this.boundingBox.maxY, this.boundingBox.maxZ - 1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.maxX - 1, this.boundingBox.minY, this.boundingBox.minZ + 1, this.boundingBox.maxX - 1, this.boundingBox.maxY, this.boundingBox.minZ + 1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.maxX - 1, this.boundingBox.minY, this.boundingBox.maxZ - 1, this.boundingBox.maxX - 1, this.boundingBox.maxY, this.boundingBox.maxZ - 1, Blocks.planks.getDefaultState(), Blocks.air.getDefaultState(), false);

                for (int i = this.boundingBox.minX; i <= this.boundingBox.maxX; ++i)
                {
                    for (int j = this.boundingBox.minZ; j <= this.boundingBox.maxZ; ++j)
                    {
                        if (this.getBlockStateFromPos(worldIn, i, this.boundingBox.minY - 1, j, structureBoundingBoxIn).getMaterial() == Material.air)
                        {
                            this.setBlockState(worldIn, Blocks.planks.getDefaultState(), i, this.boundingBox.minY - 1, j, structureBoundingBoxIn);
                        }
                    }
                }

                return true;
            }
        }
    }

    public static class Room extends StructureComponent
    {
        private List<StructureBoundingBox> roomsLinkedToTheRoom = Lists.<StructureBoundingBox>newLinkedList();

        public Room()
        {
        }

        public Room(int type, Random rand, int x, int z)
        {
            super(type);
            this.boundingBox = new StructureBoundingBox(x, 50, z, x + 7 + rand.nextInt(6), 54 + rand.nextInt(6), z + 7 + rand.nextInt(6));
        }

        public void buildComponent(StructureComponent componentIn, List<StructureComponent> listIn, Random rand)
        {
            int i = this.getComponentType();
            int k = this.boundingBox.getYSize() - 3 - 1;

            if (k <= 0)
            {
                k = 1;
            }

            int l;

            for (int j = 0; j < this.boundingBox.getXSize(); j = l + 4)
            {
                l = j + rand.nextInt(this.boundingBox.getXSize());

                if (l + 3 > this.boundingBox.getXSize())
                {
                    break;
                }

                StructureComponent structurecomponent = StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + l, this.boundingBox.minY + rand.nextInt(k) + 1, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);

                if (structurecomponent != null)
                {
                    StructureBoundingBox structureboundingbox = structurecomponent.getBoundingBox();
                    this.roomsLinkedToTheRoom.add(new StructureBoundingBox(structureboundingbox.minX, structureboundingbox.minY, this.boundingBox.minZ, structureboundingbox.maxX, structureboundingbox.maxY, this.boundingBox.minZ + 1));
                }
            }

            for (int i1 = 0; i1 < this.boundingBox.getXSize(); i1 = l + 4)
            {
                l = i1 + rand.nextInt(this.boundingBox.getXSize());

                if (l + 3 > this.boundingBox.getXSize())
                {
                    break;
                }

                StructureComponent structurecomponent1 = StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX + l, this.boundingBox.minY + rand.nextInt(k) + 1, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);

                if (structurecomponent1 != null)
                {
                    StructureBoundingBox structureboundingbox1 = structurecomponent1.getBoundingBox();
                    this.roomsLinkedToTheRoom.add(new StructureBoundingBox(structureboundingbox1.minX, structureboundingbox1.minY, this.boundingBox.maxZ - 1, structureboundingbox1.maxX, structureboundingbox1.maxY, this.boundingBox.maxZ));
                }
            }

            for (int j1 = 0; j1 < this.boundingBox.getZSize(); j1 = l + 4)
            {
                l = j1 + rand.nextInt(this.boundingBox.getZSize());

                if (l + 3 > this.boundingBox.getZSize())
                {
                    break;
                }

                StructureComponent structurecomponent2 = StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY + rand.nextInt(k) + 1, this.boundingBox.minZ + l, EnumFacing.WEST, i);

                if (structurecomponent2 != null)
                {
                    StructureBoundingBox structureboundingbox2 = structurecomponent2.getBoundingBox();
                    this.roomsLinkedToTheRoom.add(new StructureBoundingBox(this.boundingBox.minX, structureboundingbox2.minY, structureboundingbox2.minZ, this.boundingBox.minX + 1, structureboundingbox2.maxY, structureboundingbox2.maxZ));
                }
            }

            for (int k1 = 0; k1 < this.boundingBox.getZSize(); k1 = l + 4)
            {
                l = k1 + rand.nextInt(this.boundingBox.getZSize());

                if (l + 3 > this.boundingBox.getZSize())
                {
                    break;
                }

                StructureComponent structurecomponent3 = StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY + rand.nextInt(k) + 1, this.boundingBox.minZ + l, EnumFacing.EAST, i);

                if (structurecomponent3 != null)
                {
                    StructureBoundingBox structureboundingbox3 = structurecomponent3.getBoundingBox();
                    this.roomsLinkedToTheRoom.add(new StructureBoundingBox(this.boundingBox.maxX - 1, structureboundingbox3.minY, structureboundingbox3.minZ, this.boundingBox.maxX, structureboundingbox3.maxY, structureboundingbox3.maxZ));
                }
            }
        }

        public boolean addComponentParts(World worldIn, Random randomIn, StructureBoundingBox structureBoundingBoxIn)
        {
            if (this.isLiquidInStructureBoundingBox(worldIn, structureBoundingBoxIn))
            {
                return false;
            }
            else
            {
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.minY, this.boundingBox.minZ, this.boundingBox.maxX, this.boundingBox.minY, this.boundingBox.maxZ, Blocks.dirt.getDefaultState(), Blocks.air.getDefaultState(), true);
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.minY + 1, this.boundingBox.minZ, this.boundingBox.maxX, Math.min(this.boundingBox.minY + 3, this.boundingBox.maxY), this.boundingBox.maxZ, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);

                for (StructureBoundingBox structureboundingbox : this.roomsLinkedToTheRoom)
                {
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, structureboundingbox.minX, structureboundingbox.maxY - 2, structureboundingbox.minZ, structureboundingbox.maxX, structureboundingbox.maxY, structureboundingbox.maxZ, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                }

                this.randomlyRareFillWithBlocks(worldIn, structureBoundingBoxIn, this.boundingBox.minX, this.boundingBox.minY + 4, this.boundingBox.minZ, this.boundingBox.maxX, this.boundingBox.maxY, this.boundingBox.maxZ, Blocks.air.getDefaultState(), false);
                return true;
            }
        }

        public void func_181138_a(int p_181138_1_, int p_181138_2_, int p_181138_3_)
        {
            super.func_181138_a(p_181138_1_, p_181138_2_, p_181138_3_);

            for (StructureBoundingBox structureboundingbox : this.roomsLinkedToTheRoom)
            {
                structureboundingbox.offset(p_181138_1_, p_181138_2_, p_181138_3_);
            }
        }

        protected void writeStructureToNBT(NBTTagCompound tagCompound)
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (StructureBoundingBox structureboundingbox : this.roomsLinkedToTheRoom)
            {
                nbttaglist.appendTag(structureboundingbox.toNBTTagIntArray());
            }

            tagCompound.setTag("Entrances", nbttaglist);
        }

        protected void readStructureFromNBT(NBTTagCompound tagCompound)
        {
            NBTTagList nbttaglist = tagCompound.getTagList("Entrances", 11);

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                this.roomsLinkedToTheRoom.add(new StructureBoundingBox(nbttaglist.getIntArrayAt(i)));
            }
        }
    }

    public static class Stairs extends StructureComponent
    {
        public Stairs()
        {
        }

        public Stairs(int type, Random rand, StructureBoundingBox structurebb, EnumFacing facing)
        {
            super(type);
            this.func_186164_a(facing);
            this.boundingBox = structurebb;
        }

        protected void writeStructureToNBT(NBTTagCompound tagCompound)
        {
        }

        protected void readStructureFromNBT(NBTTagCompound tagCompound)
        {
        }

        public static StructureBoundingBox func_175812_a(List<StructureComponent> listIn, Random rand, int x, int y, int z, EnumFacing facing)
        {
            StructureBoundingBox structureboundingbox = new StructureBoundingBox(x, y - 5, z, x, y + 2, z);

            switch (facing)
            {
                case NORTH:
                    structureboundingbox.maxX = x + 2;
                    structureboundingbox.minZ = z - 8;
                    break;

                case SOUTH:
                    structureboundingbox.maxX = x + 2;
                    structureboundingbox.maxZ = z + 8;
                    break;

                case WEST:
                    structureboundingbox.minX = x - 8;
                    structureboundingbox.maxZ = z + 2;
                    break;

                case EAST:
                    structureboundingbox.maxX = x + 8;
                    structureboundingbox.maxZ = z + 2;
            }

            return StructureComponent.findIntersecting(listIn, structureboundingbox) != null ? null : structureboundingbox;
        }

        public void buildComponent(StructureComponent componentIn, List<StructureComponent> listIn, Random rand)
        {
            int i = this.getComponentType();
            EnumFacing enumfacing = this.func_186165_e();

            if (enumfacing != null)
            {
                switch (enumfacing)
                {
                    case NORTH:
                        StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY, this.boundingBox.minZ - 1, EnumFacing.NORTH, i);
                        break;

                    case SOUTH:
                        StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX, this.boundingBox.minY, this.boundingBox.maxZ + 1, EnumFacing.SOUTH, i);
                        break;

                    case WEST:
                        StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.minX - 1, this.boundingBox.minY, this.boundingBox.minZ, EnumFacing.WEST, i);
                        break;

                    case EAST:
                        StructureMineshaftPieces.func_175890_b(componentIn, listIn, rand, this.boundingBox.maxX + 1, this.boundingBox.minY, this.boundingBox.minZ, EnumFacing.EAST, i);
                }
            }
        }

        public boolean addComponentParts(World worldIn, Random randomIn, StructureBoundingBox structureBoundingBoxIn)
        {
            if (this.isLiquidInStructureBoundingBox(worldIn, structureBoundingBoxIn))
            {
                return false;
            }
            else
            {
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 5, 0, 2, 7, 1, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 0, 7, 2, 2, 8, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);

                for (int i = 0; i < 5; ++i)
                {
                    this.fillWithBlocks(worldIn, structureBoundingBoxIn, 0, 5 - i - (i < 4 ? 1 : 0), 2 + i, 2, 7 - i, 2 + i, Blocks.air.getDefaultState(), Blocks.air.getDefaultState(), false);
                }

                return true;
            }
        }
    }
}
