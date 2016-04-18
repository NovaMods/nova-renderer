package net.minecraft.world.gen.structure.template;

import com.google.common.base.Predicate;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.item.EntityPainting;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.inventory.IInventory;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagDouble;
import net.minecraft.nbt.NBTTagInt;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityStructure;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.gen.structure.StructureBoundingBox;

public class Template
{
    private final List<Template.BlockInfo> field_186270_a = Lists.<Template.BlockInfo>newArrayList();
    private final List<Template.EntityInfo> field_186271_b = Lists.<Template.EntityInfo>newArrayList();
    private BlockPos field_186272_c = BlockPos.ORIGIN;
    private String field_186273_d = "?";

    public BlockPos func_186259_a()
    {
        return this.field_186272_c;
    }

    public void func_186252_a(String p_186252_1_)
    {
        this.field_186273_d = p_186252_1_;
    }

    public String func_186261_b()
    {
        return this.field_186273_d;
    }

    public void func_186254_a(World p_186254_1_, BlockPos p_186254_2_, BlockPos p_186254_3_, boolean p_186254_4_, Block p_186254_5_)
    {
        if (p_186254_3_.getX() >= 1 && p_186254_3_.getY() >= 1 && p_186254_3_.getZ() >= 1)
        {
            BlockPos blockpos = p_186254_2_.add(p_186254_3_).add(-1, -1, -1);
            List<Template.BlockInfo> list = Lists.<Template.BlockInfo>newArrayList();
            List<Template.BlockInfo> list1 = Lists.<Template.BlockInfo>newArrayList();
            List<Template.BlockInfo> list2 = Lists.<Template.BlockInfo>newArrayList();
            BlockPos blockpos1 = new BlockPos(Math.min(p_186254_2_.getX(), blockpos.getX()), Math.min(p_186254_2_.getY(), blockpos.getY()), Math.min(p_186254_2_.getZ(), blockpos.getZ()));
            BlockPos blockpos2 = new BlockPos(Math.max(p_186254_2_.getX(), blockpos.getX()), Math.max(p_186254_2_.getY(), blockpos.getY()), Math.max(p_186254_2_.getZ(), blockpos.getZ()));
            this.field_186272_c = p_186254_3_;

            for (BlockPos.MutableBlockPos blockpos$mutableblockpos : BlockPos.getAllInBoxMutable(blockpos1, blockpos2))
            {
                BlockPos blockpos3 = blockpos$mutableblockpos.subtract(blockpos1);
                IBlockState iblockstate = p_186254_1_.getBlockState(blockpos$mutableblockpos);

                if (p_186254_5_ == null || p_186254_5_ != iblockstate.getBlock())
                {
                    TileEntity tileentity = p_186254_1_.getTileEntity(blockpos$mutableblockpos);

                    if (tileentity != null)
                    {
                        NBTTagCompound nbttagcompound = new NBTTagCompound();
                        tileentity.writeToNBT(nbttagcompound);
                        nbttagcompound.removeTag("x");
                        nbttagcompound.removeTag("y");
                        nbttagcompound.removeTag("z");
                        list1.add(new Template.BlockInfo(blockpos3, iblockstate, nbttagcompound));
                    }
                    else if (!iblockstate.isFullBlock() && !iblockstate.isFullCube())
                    {
                        list2.add(new Template.BlockInfo(blockpos3, iblockstate, (NBTTagCompound)null));
                    }
                    else
                    {
                        list.add(new Template.BlockInfo(blockpos3, iblockstate, (NBTTagCompound)null));
                    }
                }
            }

            this.field_186270_a.clear();
            this.field_186270_a.addAll(list);
            this.field_186270_a.addAll(list1);
            this.field_186270_a.addAll(list2);

            if (p_186254_4_)
            {
                this.func_186255_a(p_186254_1_, blockpos1, blockpos2.add(1, 1, 1));
            }
            else
            {
                this.field_186271_b.clear();
            }
        }
    }

    private void func_186255_a(World p_186255_1_, BlockPos p_186255_2_, BlockPos p_186255_3_)
    {
        List<Entity> list = p_186255_1_.<Entity>getEntitiesWithinAABB(Entity.class, new AxisAlignedBB(p_186255_2_, p_186255_3_), new Predicate<Entity>()
        {
            public boolean apply(Entity p_apply_1_)
            {
                return !(p_apply_1_ instanceof EntityPlayer);
            }
        });
        this.field_186271_b.clear();

        for (Entity entity : list)
        {
            Vec3d vec3d = new Vec3d(entity.posX - (double)p_186255_2_.getX(), entity.posY - (double)p_186255_2_.getY(), entity.posZ - (double)p_186255_2_.getZ());
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            entity.writeToNBTOptional(nbttagcompound);
            BlockPos blockpos;

            if (entity instanceof EntityPainting)
            {
                blockpos = ((EntityPainting)entity).getHangingPosition().subtract(p_186255_2_);
            }
            else
            {
                blockpos = new BlockPos(vec3d);
            }

            this.field_186271_b.add(new Template.EntityInfo(vec3d, blockpos, nbttagcompound));
        }
    }

    public Map<BlockPos, String> func_186258_a(BlockPos p_186258_1_, PlacementSettings p_186258_2_)
    {
        Map<BlockPos, String> map = Maps.<BlockPos, String>newHashMap();
        StructureBoundingBox structureboundingbox = p_186258_2_.func_186213_g();

        for (Template.BlockInfo template$blockinfo : this.field_186270_a)
        {
            BlockPos blockpos = func_186266_a(p_186258_2_, template$blockinfo.field_186242_a).add(p_186258_1_);

            if (structureboundingbox == null || structureboundingbox.isVecInside(blockpos))
            {
                IBlockState iblockstate = template$blockinfo.field_186243_b;

                if (iblockstate.getBlock() == Blocks.structure_block && template$blockinfo.field_186244_c != null)
                {
                    TileEntityStructure.Mode tileentitystructure$mode = TileEntityStructure.Mode.valueOf(template$blockinfo.field_186244_c.getString("mode"));

                    if (tileentitystructure$mode == TileEntityStructure.Mode.DATA)
                    {
                        map.put(blockpos, template$blockinfo.field_186244_c.getString("metadata"));
                    }
                }
            }
        }

        return map;
    }

    public BlockPos func_186262_a(PlacementSettings p_186262_1_, BlockPos p_186262_2_, PlacementSettings p_186262_3_, BlockPos p_186262_4_)
    {
        BlockPos blockpos = func_186266_a(p_186262_1_, p_186262_2_);
        BlockPos blockpos1 = func_186266_a(p_186262_3_, p_186262_4_);
        return blockpos.subtract(blockpos1);
    }

    public static BlockPos func_186266_a(PlacementSettings p_186266_0_, BlockPos p_186266_1_)
    {
        return func_186268_a(p_186266_1_, p_186266_0_.func_186212_b(), p_186266_0_.func_186215_c());
    }

    public void func_186260_a(World p_186260_1_, BlockPos p_186260_2_, PlacementSettings p_186260_3_)
    {
        p_186260_3_.func_186224_i();
        this.func_186253_b(p_186260_1_, p_186260_2_, p_186260_3_);
    }

    public void func_186253_b(World p_186253_1_, BlockPos p_186253_2_, PlacementSettings p_186253_3_)
    {
        if (!this.field_186270_a.isEmpty() && this.field_186272_c.getX() >= 1 && this.field_186272_c.getY() >= 1 && this.field_186272_c.getZ() >= 1)
        {
            Block block = p_186253_3_.func_186219_f();
            StructureBoundingBox structureboundingbox = p_186253_3_.func_186213_g();

            for (Template.BlockInfo template$blockinfo : this.field_186270_a)
            {
                Block block1 = template$blockinfo.field_186243_b.getBlock();

                if ((block == null || block != block1) && (!p_186253_3_.func_186227_h() || block1 != Blocks.structure_block))
                {
                    BlockPos blockpos = func_186266_a(p_186253_3_, template$blockinfo.field_186242_a).add(p_186253_2_);

                    if (structureboundingbox == null || structureboundingbox.isVecInside(blockpos))
                    {
                        IBlockState iblockstate = template$blockinfo.field_186243_b.withMirror(p_186253_3_.func_186212_b());
                        IBlockState iblockstate1 = iblockstate.withRotation(p_186253_3_.func_186215_c());

                        if (template$blockinfo.field_186244_c != null)
                        {
                            TileEntity tileentity = p_186253_1_.getTileEntity(blockpos);

                            if (tileentity != null)
                            {
                                if (tileentity instanceof IInventory)
                                {
                                    ((IInventory)tileentity).clear();
                                }

                                p_186253_1_.setBlockState(blockpos, Blocks.barrier.getDefaultState(), 4);
                            }
                        }

                        if (p_186253_1_.setBlockState(blockpos, iblockstate1, 2) && template$blockinfo.field_186244_c != null)
                        {
                            TileEntity tileentity2 = p_186253_1_.getTileEntity(blockpos);

                            if (tileentity2 != null)
                            {
                                template$blockinfo.field_186244_c.setInteger("x", blockpos.getX());
                                template$blockinfo.field_186244_c.setInteger("y", blockpos.getY());
                                template$blockinfo.field_186244_c.setInteger("z", blockpos.getZ());
                                tileentity2.readFromNBT(template$blockinfo.field_186244_c);
                            }
                        }
                    }
                }
            }

            for (Template.BlockInfo template$blockinfo1 : this.field_186270_a)
            {
                if (block == null || block != template$blockinfo1.field_186243_b.getBlock())
                {
                    BlockPos blockpos1 = func_186266_a(p_186253_3_, template$blockinfo1.field_186242_a).add(p_186253_2_);

                    if (structureboundingbox == null || structureboundingbox.isVecInside(blockpos1))
                    {
                        p_186253_1_.notifyNeighborsRespectDebug(blockpos1, template$blockinfo1.field_186243_b.getBlock());

                        if (template$blockinfo1.field_186244_c != null)
                        {
                            TileEntity tileentity1 = p_186253_1_.getTileEntity(blockpos1);

                            if (tileentity1 != null)
                            {
                                tileentity1.markDirty();
                            }
                        }
                    }
                }
            }

            if (!p_186253_3_.func_186221_e())
            {
                this.func_186263_a(p_186253_1_, p_186253_2_, p_186253_3_.func_186212_b(), p_186253_3_.func_186215_c(), structureboundingbox);
            }
        }
    }

    private void func_186263_a(World p_186263_1_, BlockPos p_186263_2_, Mirror p_186263_3_, Rotation p_186263_4_, StructureBoundingBox p_186263_5_)
    {
        for (Template.EntityInfo template$entityinfo : this.field_186271_b)
        {
            BlockPos blockpos = func_186268_a(template$entityinfo.field_186248_b, p_186263_3_, p_186263_4_).add(p_186263_2_);

            if (p_186263_5_ == null || p_186263_5_.isVecInside(blockpos))
            {
                NBTTagCompound nbttagcompound = template$entityinfo.field_186249_c;
                Vec3d vec3d = func_186269_a(template$entityinfo.field_186247_a, p_186263_3_, p_186263_4_);
                Vec3d vec3d1 = vec3d.addVector((double)p_186263_2_.getX(), (double)p_186263_2_.getY(), (double)p_186263_2_.getZ());
                NBTTagList nbttaglist = new NBTTagList();
                nbttaglist.appendTag(new NBTTagDouble(vec3d1.xCoord));
                nbttaglist.appendTag(new NBTTagDouble(vec3d1.yCoord));
                nbttaglist.appendTag(new NBTTagDouble(vec3d1.zCoord));
                nbttagcompound.setTag("Pos", nbttaglist);
                nbttagcompound.setUniqueId("UUID", UUID.randomUUID());
                Entity entity;

                try
                {
                    entity = EntityList.createEntityFromNBT(nbttagcompound, p_186263_1_);
                }
                catch (Exception var15)
                {
                    entity = null;
                }

                if (entity != null)
                {
                    if (entity instanceof EntityPainting)
                    {
                        entity.func_184217_a(p_186263_3_);
                        entity.func_184229_a(p_186263_4_);
                        entity.setPosition((double)blockpos.getX(), (double)blockpos.getY(), (double)blockpos.getZ());
                        entity.setLocationAndAngles(vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, entity.rotationYaw, entity.rotationPitch);
                    }
                    else
                    {
                        float f = entity.func_184217_a(p_186263_3_);
                        f = f + (entity.rotationYaw - entity.func_184229_a(p_186263_4_));
                        entity.setLocationAndAngles(vec3d1.xCoord, vec3d1.yCoord, vec3d1.zCoord, f, entity.rotationPitch);
                    }

                    p_186263_1_.spawnEntityInWorld(entity);
                }
            }
        }
    }

    public BlockPos func_186257_a(Rotation p_186257_1_)
    {
        switch (p_186257_1_)
        {
            case COUNTERCLOCKWISE_90:
            case CLOCKWISE_90:
                return new BlockPos(this.field_186272_c.getZ(), this.field_186272_c.getY(), this.field_186272_c.getX());

            default:
                return this.field_186272_c;
        }
    }

    private static BlockPos func_186268_a(BlockPos p_186268_0_, Mirror p_186268_1_, Rotation p_186268_2_)
    {
        int i = p_186268_0_.getX();
        int j = p_186268_0_.getY();
        int k = p_186268_0_.getZ();
        boolean flag = true;

        switch (p_186268_1_)
        {
            case LEFT_RIGHT:
                k = -k;
                break;

            case FRONT_BACK:
                i = -i;
                break;

            default:
                flag = false;
        }

        switch (p_186268_2_)
        {
            case COUNTERCLOCKWISE_90:
                return new BlockPos(k, j, -i);

            case CLOCKWISE_90:
                return new BlockPos(-k, j, i);

            case CLOCKWISE_180:
                return new BlockPos(-i, j, -k);

            default:
                return flag ? new BlockPos(i, j, k) : p_186268_0_;
        }
    }

    private static Vec3d func_186269_a(Vec3d p_186269_0_, Mirror p_186269_1_, Rotation p_186269_2_)
    {
        double d0 = p_186269_0_.xCoord;
        double d1 = p_186269_0_.yCoord;
        double d2 = p_186269_0_.zCoord;
        boolean flag = true;

        switch (p_186269_1_)
        {
            case LEFT_RIGHT:
                d2 = 1.0D - d2;
                break;

            case FRONT_BACK:
                d0 = 1.0D - d0;
                break;

            default:
                flag = false;
        }

        switch (p_186269_2_)
        {
            case COUNTERCLOCKWISE_90:
                return new Vec3d(d2, d1, 1.0D - d0);

            case CLOCKWISE_90:
                return new Vec3d(1.0D - d2, d1, d0);

            case CLOCKWISE_180:
                return new Vec3d(1.0D - d0, d1, 1.0D - d2);

            default:
                return flag ? new Vec3d(d0, d1, d2) : p_186269_0_;
        }
    }

    public void write(NBTTagCompound compound)
    {
        NBTTagList nbttaglist = new NBTTagList();

        for (Template.BlockInfo template$blockinfo : this.field_186270_a)
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            nbttagcompound.setTag("pos", this.func_186267_a(new int[] {template$blockinfo.field_186242_a.getX(), template$blockinfo.field_186242_a.getY(), template$blockinfo.field_186242_a.getZ()}));
            nbttagcompound.setInteger("state", Block.getStateId(template$blockinfo.field_186243_b));

            if (template$blockinfo.field_186244_c != null)
            {
                nbttagcompound.setTag("nbt", template$blockinfo.field_186244_c);
            }

            nbttaglist.appendTag(nbttagcompound);
        }

        NBTTagList nbttaglist1 = new NBTTagList();

        for (Template.EntityInfo template$entityinfo : this.field_186271_b)
        {
            NBTTagCompound nbttagcompound1 = new NBTTagCompound();
            nbttagcompound1.setTag("pos", this.func_186264_a(new double[] {template$entityinfo.field_186247_a.xCoord, template$entityinfo.field_186247_a.yCoord, template$entityinfo.field_186247_a.zCoord}));
            nbttagcompound1.setTag("blockPos", this.func_186267_a(new int[] {template$entityinfo.field_186248_b.getX(), template$entityinfo.field_186248_b.getY(), template$entityinfo.field_186248_b.getZ()}));

            if (template$entityinfo.field_186249_c != null)
            {
                nbttagcompound1.setTag("nbt", template$entityinfo.field_186249_c);
            }

            nbttaglist1.appendTag(nbttagcompound1);
        }

        compound.setTag("blocks", nbttaglist);
        compound.setTag("entities", nbttaglist1);
        compound.setTag("size", this.func_186267_a(new int[] {this.field_186272_c.getX(), this.field_186272_c.getY(), this.field_186272_c.getZ()}));
        compound.setInteger("version", 1);
        compound.setString("author", this.field_186273_d);
    }

    public void read(NBTTagCompound compound)
    {
        this.field_186270_a.clear();
        this.field_186271_b.clear();
        NBTTagList nbttaglist = compound.getTagList("size", 3);
        this.field_186272_c = new BlockPos(nbttaglist.getIntAt(0), nbttaglist.getIntAt(1), nbttaglist.getIntAt(2));
        this.field_186273_d = compound.getString("author");
        NBTTagList nbttaglist1 = compound.getTagList("blocks", 10);

        for (int i = 0; i < nbttaglist1.tagCount(); ++i)
        {
            NBTTagCompound nbttagcompound = nbttaglist1.getCompoundTagAt(i);
            NBTTagList nbttaglist2 = nbttagcompound.getTagList("pos", 3);
            BlockPos blockpos = new BlockPos(nbttaglist2.getIntAt(0), nbttaglist2.getIntAt(1), nbttaglist2.getIntAt(2));
            int j = nbttagcompound.getInteger("state");
            IBlockState iblockstate = Block.getStateById(j);
            NBTTagCompound nbttagcompound1;

            if (nbttagcompound.hasKey("nbt"))
            {
                nbttagcompound1 = nbttagcompound.getCompoundTag("nbt");
            }
            else
            {
                nbttagcompound1 = null;
            }

            this.field_186270_a.add(new Template.BlockInfo(blockpos, iblockstate, nbttagcompound1));
        }

        NBTTagList nbttaglist3 = compound.getTagList("entities", 10);

        for (int k = 0; k < nbttaglist3.tagCount(); ++k)
        {
            NBTTagCompound nbttagcompound3 = nbttaglist3.getCompoundTagAt(k);
            NBTTagList nbttaglist4 = nbttagcompound3.getTagList("pos", 6);
            Vec3d vec3d = new Vec3d(nbttaglist4.getDoubleAt(0), nbttaglist4.getDoubleAt(1), nbttaglist4.getDoubleAt(2));
            NBTTagList nbttaglist5 = nbttagcompound3.getTagList("blockPos", 3);
            BlockPos blockpos1 = new BlockPos(nbttaglist5.getIntAt(0), nbttaglist5.getIntAt(1), nbttaglist5.getIntAt(2));

            if (nbttagcompound3.hasKey("nbt"))
            {
                NBTTagCompound nbttagcompound2 = nbttagcompound3.getCompoundTag("nbt");
                this.field_186271_b.add(new Template.EntityInfo(vec3d, blockpos1, nbttagcompound2));
            }
        }
    }

    private NBTTagList func_186267_a(int... p_186267_1_)
    {
        NBTTagList nbttaglist = new NBTTagList();

        for (int i : p_186267_1_)
        {
            nbttaglist.appendTag(new NBTTagInt(i));
        }

        return nbttaglist;
    }

    private NBTTagList func_186264_a(double... p_186264_1_)
    {
        NBTTagList nbttaglist = new NBTTagList();

        for (double d0 : p_186264_1_)
        {
            nbttaglist.appendTag(new NBTTagDouble(d0));
        }

        return nbttaglist;
    }

    static class BlockInfo
    {
        public final BlockPos field_186242_a;
        public final IBlockState field_186243_b;
        public final NBTTagCompound field_186244_c;

        private BlockInfo(BlockPos p_i47042_1_, IBlockState p_i47042_2_, NBTTagCompound p_i47042_3_)
        {
            this.field_186242_a = p_i47042_1_;
            this.field_186243_b = p_i47042_2_;
            this.field_186244_c = p_i47042_3_;
        }
    }

    static class EntityInfo
    {
        public final Vec3d field_186247_a;
        public final BlockPos field_186248_b;
        public final NBTTagCompound field_186249_c;

        private EntityInfo(Vec3d p_i47101_1_, BlockPos p_i47101_2_, NBTTagCompound p_i47101_3_)
        {
            this.field_186247_a = p_i47101_1_;
            this.field_186248_b = p_i47101_2_;
            this.field_186249_c = p_i47101_3_;
        }
    }
}
