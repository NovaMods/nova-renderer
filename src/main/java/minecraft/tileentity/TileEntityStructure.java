package net.minecraft.tileentity;

import com.google.common.base.Predicate;
import com.google.common.collect.Iterables;
import com.google.common.collect.Lists;
import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.block.BlockStructure;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.IStringSerializable;
import net.minecraft.util.Mirror;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.Rotation;
import net.minecraft.util.StringUtils;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.WorldServer;
import net.minecraft.world.gen.structure.StructureBoundingBox;
import net.minecraft.world.gen.structure.template.PlacementSettings;
import net.minecraft.world.gen.structure.template.Template;
import net.minecraft.world.gen.structure.template.TemplateManager;

public class TileEntityStructure extends TileEntity
{
    private String name = "";
    private String author = "";
    private String metadata = "";
    private BlockPos position = new BlockPos(1, 1, 1);
    private BlockPos size = BlockPos.ORIGIN;
    private Mirror mirror = Mirror.NONE;
    private Rotation rotation = Rotation.NONE;
    private TileEntityStructure.Mode mode = TileEntityStructure.Mode.DATA;
    private boolean ignoreEntities;

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        compound.setString("name", this.name);
        compound.setString("author", this.author);
        compound.setString("metadata", this.metadata);
        compound.setInteger("posX", this.position.getX());
        compound.setInteger("posY", this.position.getY());
        compound.setInteger("posZ", this.position.getZ());
        compound.setInteger("sizeX", this.size.getX());
        compound.setInteger("sizeY", this.size.getY());
        compound.setInteger("sizeZ", this.size.getZ());
        compound.setString("rotation", this.rotation.toString());
        compound.setString("mirror", this.mirror.toString());
        compound.setString("mode", this.mode.toString());
        compound.setBoolean("ignoreEntities", this.ignoreEntities);
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.name = compound.getString("name");
        this.author = compound.getString("author");
        this.metadata = compound.getString("metadata");
        this.position = new BlockPos(compound.getInteger("posX"), compound.getInteger("posY"), compound.getInteger("posZ"));
        this.size = new BlockPos(compound.getInteger("sizeX"), compound.getInteger("sizeY"), compound.getInteger("sizeZ"));

        try
        {
            this.rotation = Rotation.valueOf(compound.getString("rotation"));
        }
        catch (IllegalArgumentException var5)
        {
            this.rotation = Rotation.NONE;
        }

        try
        {
            this.mirror = Mirror.valueOf(compound.getString("mirror"));
        }
        catch (IllegalArgumentException var4)
        {
            this.mirror = Mirror.NONE;
        }

        try
        {
            this.mode = TileEntityStructure.Mode.valueOf(compound.getString("mode"));
        }
        catch (IllegalArgumentException var3)
        {
            this.mode = TileEntityStructure.Mode.DATA;
        }

        this.ignoreEntities = compound.getBoolean("ignoreEntities");
    }

    public Packet<?> getDescriptionPacket()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        this.writeToNBT(nbttagcompound);
        return new SPacketUpdateTileEntity(this.pos, 7, nbttagcompound);
    }

    public void setName(String nameIn)
    {
        this.name = nameIn;
    }

    public void setPosition(BlockPos posIn)
    {
        this.position = posIn;
    }

    public void setSize(BlockPos sizeIn)
    {
        this.size = sizeIn;
    }

    public void setMirror(Mirror mirrorIn)
    {
        this.mirror = mirrorIn;
    }

    public void setRotation(Rotation rotationIn)
    {
        this.rotation = rotationIn;
    }

    public void setMetadata(String metadataIn)
    {
        this.metadata = metadataIn;
    }

    public void setMode(TileEntityStructure.Mode modeIn)
    {
        this.mode = modeIn;
        IBlockState iblockstate = this.worldObj.getBlockState(this.getPos());

        if (iblockstate.getBlock() == Blocks.structure_block)
        {
            this.worldObj.setBlockState(this.getPos(), iblockstate.withProperty(BlockStructure.MODE, modeIn), 2);
        }
    }

    public void setIgnoresEntities(boolean ignoreEntitiesIn)
    {
        this.ignoreEntities = ignoreEntitiesIn;
    }

    public boolean func_184417_l()
    {
        if (this.mode != TileEntityStructure.Mode.SAVE)
        {
            return false;
        }
        else
        {
            BlockPos blockpos = this.getPos();
            int i = 128;
            BlockPos blockpos1 = new BlockPos(blockpos.getX() - 128, 0, blockpos.getZ() - 128);
            BlockPos blockpos2 = new BlockPos(blockpos.getX() + 128, 255, blockpos.getZ() + 128);
            List<TileEntityStructure> list = this.func_184418_a(blockpos1, blockpos2);
            List<TileEntityStructure> list1 = this.func_184415_a(list);

            if (list1.size() < 1)
            {
                return false;
            }
            else
            {
                StructureBoundingBox structureboundingbox = this.func_184416_a(blockpos, list1);

                if (structureboundingbox.maxX - structureboundingbox.minX > 1 && structureboundingbox.maxY - structureboundingbox.minY > 1 && structureboundingbox.maxZ - structureboundingbox.minZ > 1)
                {
                    this.position = new BlockPos(structureboundingbox.minX - blockpos.getX() + 1, structureboundingbox.minY - blockpos.getY() + 1, structureboundingbox.minZ - blockpos.getZ() + 1);
                    this.size = new BlockPos(structureboundingbox.maxX - structureboundingbox.minX - 1, structureboundingbox.maxY - structureboundingbox.minY - 1, structureboundingbox.maxZ - structureboundingbox.minZ - 1);
                    this.markDirty();
                    IBlockState iblockstate = this.worldObj.getBlockState(blockpos);
                    this.worldObj.notifyBlockUpdate(blockpos, iblockstate, iblockstate, 3);
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }

    private List<TileEntityStructure> func_184415_a(List<TileEntityStructure> p_184415_1_)
    {
        Iterable<TileEntityStructure> iterable = Iterables.filter(p_184415_1_, new Predicate<TileEntityStructure>()
        {
            public boolean apply(TileEntityStructure p_apply_1_)
            {
                return p_apply_1_.mode == TileEntityStructure.Mode.CORNER && TileEntityStructure.this.name.equals(p_apply_1_.name);
            }
        });
        return Lists.newArrayList(iterable);
    }

    private List<TileEntityStructure> func_184418_a(BlockPos p_184418_1_, BlockPos p_184418_2_)
    {
        List<TileEntityStructure> list = Lists.<TileEntityStructure>newArrayList();

        for (BlockPos.MutableBlockPos blockpos$mutableblockpos : BlockPos.getAllInBoxMutable(p_184418_1_, p_184418_2_))
        {
            IBlockState iblockstate = this.worldObj.getBlockState(blockpos$mutableblockpos);

            if (iblockstate.getBlock() == Blocks.structure_block)
            {
                TileEntity tileentity = this.worldObj.getTileEntity(blockpos$mutableblockpos);

                if (tileentity != null && tileentity instanceof TileEntityStructure)
                {
                    list.add((TileEntityStructure)tileentity);
                }
            }
        }

        return list;
    }

    private StructureBoundingBox func_184416_a(BlockPos p_184416_1_, List<TileEntityStructure> p_184416_2_)
    {
        StructureBoundingBox structureboundingbox;

        if (p_184416_2_.size() > 1)
        {
            BlockPos blockpos = ((TileEntityStructure)p_184416_2_.get(0)).getPos();
            structureboundingbox = new StructureBoundingBox(blockpos, blockpos);
        }
        else
        {
            structureboundingbox = new StructureBoundingBox(p_184416_1_, p_184416_1_);
        }

        for (TileEntityStructure tileentitystructure : p_184416_2_)
        {
            BlockPos blockpos1 = tileentitystructure.getPos();

            if (blockpos1.getX() < structureboundingbox.minX)
            {
                structureboundingbox.minX = blockpos1.getX();
            }
            else if (blockpos1.getX() > structureboundingbox.maxX)
            {
                structureboundingbox.maxX = blockpos1.getX();
            }

            if (blockpos1.getY() < structureboundingbox.minY)
            {
                structureboundingbox.minY = blockpos1.getY();
            }
            else if (blockpos1.getY() > structureboundingbox.maxY)
            {
                structureboundingbox.maxY = blockpos1.getY();
            }

            if (blockpos1.getZ() < structureboundingbox.minZ)
            {
                structureboundingbox.minZ = blockpos1.getZ();
            }
            else if (blockpos1.getZ() > structureboundingbox.maxZ)
            {
                structureboundingbox.maxZ = blockpos1.getZ();
            }
        }

        return structureboundingbox;
    }

    public boolean func_184419_m()
    {
        if (this.mode == TileEntityStructure.Mode.SAVE && !this.worldObj.isRemote)
        {
            BlockPos blockpos = this.getPos().add(this.position);
            WorldServer worldserver = (WorldServer)this.worldObj;
            MinecraftServer minecraftserver = this.worldObj.getMinecraftServer();
            TemplateManager templatemanager = worldserver.getStructureTemplateManager();
            Template template = templatemanager.getTemplate(minecraftserver, new ResourceLocation(this.name));
            template.func_186254_a(this.worldObj, blockpos, this.size, !this.ignoreEntities, Blocks.barrier);
            template.func_186252_a(this.author);
            templatemanager.func_186238_c(minecraftserver, new ResourceLocation(this.name));
            return true;
        }
        else
        {
            return false;
        }
    }

    public boolean func_184412_n()
    {
        if (this.mode == TileEntityStructure.Mode.LOAD && !this.worldObj.isRemote)
        {
            BlockPos blockpos = this.getPos().add(this.position);
            WorldServer worldserver = (WorldServer)this.worldObj;
            MinecraftServer minecraftserver = this.worldObj.getMinecraftServer();
            TemplateManager templatemanager = worldserver.getStructureTemplateManager();
            Template template = templatemanager.getTemplate(minecraftserver, new ResourceLocation(this.name));

            if (!StringUtils.isNullOrEmpty(template.func_186261_b()))
            {
                this.author = template.func_186261_b();
            }

            if (!this.size.equals(template.func_186259_a()))
            {
                this.size = template.func_186259_a();
                return false;
            }
            else
            {
                BlockPos blockpos1 = template.func_186257_a(this.rotation);

                for (Entity entity : this.worldObj.getEntitiesWithinAABBExcludingEntity((Entity)null, new AxisAlignedBB(blockpos, blockpos1.add(blockpos).add(-1, -1, -1))))
                {
                    this.worldObj.removePlayerEntityDangerously(entity);
                }

                PlacementSettings placementsettings = (new PlacementSettings()).func_186214_a(this.mirror).func_186220_a(this.rotation).func_186222_a(this.ignoreEntities).func_186218_a((ChunkCoordIntPair)null).func_186225_a((Block)null).func_186226_b(false);
                template.func_186260_a(this.worldObj, blockpos, placementsettings);
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    public static enum Mode implements IStringSerializable
    {
        SAVE("save", 0),
        LOAD("load", 1),
        CORNER("corner", 2),
        DATA("data", 3);

        private static final TileEntityStructure.Mode[] field_185115_e = new TileEntityStructure.Mode[values().length];
        private final String field_185116_f;
        private final int field_185117_g;

        private Mode(String p_i47027_3_, int p_i47027_4_)
        {
            this.field_185116_f = p_i47027_3_;
            this.field_185117_g = p_i47027_4_;
        }

        public String getName()
        {
            return this.field_185116_f;
        }

        public int func_185110_a()
        {
            return this.field_185117_g;
        }

        public static TileEntityStructure.Mode func_185108_a(int p_185108_0_)
        {
            if (p_185108_0_ < 0 || p_185108_0_ >= field_185115_e.length)
            {
                p_185108_0_ = 0;
            }

            return field_185115_e[p_185108_0_];
        }

        static {
            for (TileEntityStructure.Mode tileentitystructure$mode : values())
            {
                field_185115_e[tileentitystructure$mode.func_185110_a()] = tileentitystructure$mode;
            }
        }
    }
}
