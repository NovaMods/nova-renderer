package net.minecraft.tileentity;

import net.minecraft.block.state.IBlockState;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.util.ITickable;
import net.minecraft.util.WeightedSpawnerEntity;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;

public class TileEntityMobSpawner extends TileEntity implements ITickable
{
    private final MobSpawnerBaseLogic spawnerLogic = new MobSpawnerBaseLogic()
    {
        public void func_98267_a(int id)
        {
            TileEntityMobSpawner.this.worldObj.addBlockEvent(TileEntityMobSpawner.this.pos, Blocks.mob_spawner, id, 0);
        }
        public World getSpawnerWorld()
        {
            return TileEntityMobSpawner.this.worldObj;
        }
        public BlockPos getSpawnerPosition()
        {
            return TileEntityMobSpawner.this.pos;
        }
        public void func_184993_a(WeightedSpawnerEntity p_184993_1_)
        {
            super.func_184993_a(p_184993_1_);

            if (this.getSpawnerWorld() != null)
            {
                IBlockState iblockstate = this.getSpawnerWorld().getBlockState(this.getSpawnerPosition());
                this.getSpawnerWorld().notifyBlockUpdate(TileEntityMobSpawner.this.pos, iblockstate, iblockstate, 4);
            }
        }
    };

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.spawnerLogic.readFromNBT(compound);
    }

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        this.spawnerLogic.writeToNBT(compound);
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        this.spawnerLogic.updateSpawner();
    }

    public Packet<?> getDescriptionPacket()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        this.writeToNBT(nbttagcompound);
        nbttagcompound.removeTag("SpawnPotentials");
        return new SPacketUpdateTileEntity(this.pos, 1, nbttagcompound);
    }

    public boolean receiveClientEvent(int id, int type)
    {
        return this.spawnerLogic.setDelayToMin(id) ? true : super.receiveClientEvent(id, type);
    }

    public boolean func_183000_F()
    {
        return true;
    }

    public MobSpawnerBaseLogic getSpawnerBaseLogic()
    {
        return this.spawnerLogic;
    }
}
