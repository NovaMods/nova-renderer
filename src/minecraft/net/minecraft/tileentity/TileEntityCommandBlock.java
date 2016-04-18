package net.minecraft.tileentity;

import io.netty.buffer.ByteBuf;
import net.minecraft.block.Block;
import net.minecraft.block.BlockCommandBlock;
import net.minecraft.block.state.IBlockState;
import net.minecraft.command.CommandResultStats;
import net.minecraft.entity.Entity;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.server.MinecraftServer;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;

public class TileEntityCommandBlock extends TileEntity
{
    private boolean powered;
    private boolean auto;
    private boolean conditionMet;
    private boolean field_184262_h;
    private final CommandBlockBaseLogic commandBlockLogic = new CommandBlockBaseLogic()
    {
        public BlockPos getPosition()
        {
            return TileEntityCommandBlock.this.pos;
        }
        public Vec3d getPositionVector()
        {
            return new Vec3d((double)TileEntityCommandBlock.this.pos.getX() + 0.5D, (double)TileEntityCommandBlock.this.pos.getY() + 0.5D, (double)TileEntityCommandBlock.this.pos.getZ() + 0.5D);
        }
        public World getEntityWorld()
        {
            return TileEntityCommandBlock.this.getWorld();
        }
        public void setCommand(String command)
        {
            super.setCommand(command);
            TileEntityCommandBlock.this.markDirty();
        }
        public void updateCommand()
        {
            IBlockState iblockstate = TileEntityCommandBlock.this.worldObj.getBlockState(TileEntityCommandBlock.this.pos);
            TileEntityCommandBlock.this.getWorld().notifyBlockUpdate(TileEntityCommandBlock.this.pos, iblockstate, iblockstate, 3);
        }
        public int func_145751_f()
        {
            return 0;
        }
        public void func_145757_a(ByteBuf p_145757_1_)
        {
            p_145757_1_.writeInt(TileEntityCommandBlock.this.pos.getX());
            p_145757_1_.writeInt(TileEntityCommandBlock.this.pos.getY());
            p_145757_1_.writeInt(TileEntityCommandBlock.this.pos.getZ());
        }
        public Entity getCommandSenderEntity()
        {
            return null;
        }
        public MinecraftServer getServer()
        {
            return TileEntityCommandBlock.this.worldObj.getMinecraftServer();
        }
    };

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        this.commandBlockLogic.writeDataToNBT(compound);
        compound.setBoolean("powered", this.isPowered());
        compound.setBoolean("conditionMet", this.isConditionMet());
        compound.setBoolean("auto", this.isAuto());
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.commandBlockLogic.readDataFromNBT(compound);
        this.setPowered(compound.getBoolean("powered"));
        this.setConditionMet(compound.getBoolean("conditionMet"));
        this.setAuto(compound.getBoolean("auto"));
    }

    public Packet<?> getDescriptionPacket()
    {
        if (this.func_184257_h())
        {
            this.func_184252_d(false);
            NBTTagCompound nbttagcompound = new NBTTagCompound();
            this.writeToNBT(nbttagcompound);
            return new SPacketUpdateTileEntity(this.pos, 2, nbttagcompound);
        }
        else
        {
            return null;
        }
    }

    public boolean func_183000_F()
    {
        return true;
    }

    public CommandBlockBaseLogic getCommandBlockLogic()
    {
        return this.commandBlockLogic;
    }

    public CommandResultStats getCommandResultStats()
    {
        return this.commandBlockLogic.getCommandResultStats();
    }

    public void setPowered(boolean p_184250_1_)
    {
        this.powered = p_184250_1_;
    }

    public boolean isPowered()
    {
        return this.powered;
    }

    public boolean isAuto()
    {
        return this.auto;
    }

    public void setAuto(boolean p_184253_1_)
    {
        boolean flag = this.auto;
        this.auto = p_184253_1_;

        if (!flag && p_184253_1_ && !this.powered && this.worldObj != null && this.func_184251_i() != TileEntityCommandBlock.Mode.SEQUENCE)
        {
            Block block = this.getBlockType();

            if (block instanceof BlockCommandBlock)
            {
                BlockPos blockpos = this.getPos();
                BlockCommandBlock blockcommandblock = (BlockCommandBlock)block;
                this.conditionMet = !this.func_184258_j() || blockcommandblock.func_185562_e(this.worldObj, blockpos, this.worldObj.getBlockState(blockpos));
                this.worldObj.scheduleUpdate(blockpos, block, block.tickRate(this.worldObj));

                if (this.conditionMet)
                {
                    blockcommandblock.func_185563_c(this.worldObj, blockpos);
                }
            }
        }
    }

    public boolean isConditionMet()
    {
        return this.conditionMet;
    }

    public void setConditionMet(boolean p_184249_1_)
    {
        this.conditionMet = p_184249_1_;
    }

    public boolean func_184257_h()
    {
        return this.field_184262_h;
    }

    public void func_184252_d(boolean p_184252_1_)
    {
        this.field_184262_h = p_184252_1_;
    }

    public TileEntityCommandBlock.Mode func_184251_i()
    {
        Block block = this.getBlockType();
        return block == Blocks.command_block ? TileEntityCommandBlock.Mode.REDSTONE : (block == Blocks.repeating_command_block ? TileEntityCommandBlock.Mode.AUTO : (block == Blocks.chain_command_block ? TileEntityCommandBlock.Mode.SEQUENCE : TileEntityCommandBlock.Mode.REDSTONE));
    }

    public boolean func_184258_j()
    {
        IBlockState iblockstate = this.worldObj.getBlockState(this.getPos());
        return iblockstate.getBlock() instanceof BlockCommandBlock ? ((Boolean)iblockstate.getValue(BlockCommandBlock.CONDITIONAL)).booleanValue() : false;
    }

    /**
     * validates a tile entity
     */
    public void validate()
    {
        this.blockType = null;
        super.validate();
    }

    public static enum Mode
    {
        SEQUENCE,
        AUTO,
        REDSTONE;
    }
}
