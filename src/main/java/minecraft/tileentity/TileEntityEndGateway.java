package net.minecraft.tileentity;

import java.util.List;
import java.util.Random;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.network.Packet;
import net.minecraft.network.play.server.SPacketUpdateTileEntity;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ITickable;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.WorldProviderEnd;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.gen.feature.WorldGenEndGateway;
import net.minecraft.world.gen.feature.WorldGenEndIsland;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class TileEntityEndGateway extends TileEntity implements ITickable
{
    private static final Logger LOGGER = LogManager.getLogger();
    private long age = 0L;
    private int field_184316_g = 0;
    private BlockPos exitPortal;
    private boolean exactTeleport;

    public void writeToNBT(NBTTagCompound compound)
    {
        super.writeToNBT(compound);
        compound.setLong("Age", this.age);

        if (this.exitPortal != null)
        {
            compound.setTag("ExitPortal", NBTUtil.createPosTag(this.exitPortal));
        }

        if (this.exactTeleport)
        {
            compound.setBoolean("ExactTeleport", this.exactTeleport);
        }
    }

    public void readFromNBT(NBTTagCompound compound)
    {
        super.readFromNBT(compound);
        this.age = compound.getLong("Age");

        if (compound.hasKey("ExitPortal", 10))
        {
            this.exitPortal = NBTUtil.getPosFromTag(compound.getCompoundTag("ExitPortal"));
        }

        this.exactTeleport = compound.getBoolean("ExactTeleport");
    }

    public double getMaxRenderDistanceSquared()
    {
        return 65536.0D;
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        boolean flag = this.func_184309_b();
        boolean flag1 = this.func_184310_d();
        ++this.age;

        if (flag1)
        {
            --this.field_184316_g;
        }
        else if (!this.worldObj.isRemote)
        {
            List<Entity> list = this.worldObj.<Entity>getEntitiesWithinAABB(Entity.class, new AxisAlignedBB(this.getPos()));

            if (!list.isEmpty())
            {
                this.func_184306_a((Entity)list.get(0));
            }
        }

        if (flag != this.func_184309_b() || flag1 != this.func_184310_d())
        {
            this.markDirty();
        }
    }

    public boolean func_184309_b()
    {
        return this.age < 200L;
    }

    public boolean func_184310_d()
    {
        return this.field_184316_g > 0;
    }

    public float func_184302_e()
    {
        return MathHelper.clamp_float((float)this.age / 200.0F, 0.0F, 1.0F);
    }

    public float func_184305_g()
    {
        return 1.0F - MathHelper.clamp_float((float)this.field_184316_g / 20.0F, 0.0F, 1.0F);
    }

    public Packet<?> getDescriptionPacket()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();
        this.writeToNBT(nbttagcompound);
        return new SPacketUpdateTileEntity(this.pos, 8, nbttagcompound);
    }

    public void func_184300_h()
    {
        if (!this.worldObj.isRemote)
        {
            this.field_184316_g = 20;
            this.worldObj.addBlockEvent(this.getPos(), this.getBlockType(), 1, 0);
            this.markDirty();
        }
    }

    public boolean receiveClientEvent(int id, int type)
    {
        if (id == 1)
        {
            this.field_184316_g = 20;
            return true;
        }
        else
        {
            return super.receiveClientEvent(id, type);
        }
    }

    public void func_184306_a(Entity p_184306_1_)
    {
        if (!this.worldObj.isRemote && !this.func_184310_d())
        {
            this.field_184316_g = 100;

            if (this.exitPortal == null && this.worldObj.provider instanceof WorldProviderEnd)
            {
                this.func_184311_k();
            }

            if (this.exitPortal != null)
            {
                BlockPos blockpos = this.exactTeleport ? this.exitPortal : this.func_184303_j();
                p_184306_1_.setPositionAndUpdate((double)blockpos.getX() + 0.5D, (double)blockpos.getY() + 0.5D, (double)blockpos.getZ() + 0.5D);
            }

            this.func_184300_h();
        }
    }

    private BlockPos func_184303_j()
    {
        BlockPos blockpos = func_184308_a(this.worldObj, this.exitPortal, 5, false);
        LOGGER.debug("Best exit position for portal at " + this.exitPortal + " is " + blockpos);
        return blockpos.up();
    }

    private void func_184311_k()
    {
        Vec3d vec3d = (new Vec3d((double)this.getPos().getX(), 0.0D, (double)this.getPos().getZ())).normalize();
        Vec3d vec3d1 = vec3d.func_186678_a(1024.0D);

        for (int i = 16; func_184301_a(this.worldObj, vec3d1).getTopFilledSegment() > 0 && i-- > 0; vec3d1 = vec3d1.add(vec3d.func_186678_a(-16.0D)))
        {
            LOGGER.debug("Skipping backwards past nonempty chunk at " + vec3d1);
        }

        for (int j = 16; func_184301_a(this.worldObj, vec3d1).getTopFilledSegment() == 0 && j-- > 0; vec3d1 = vec3d1.add(vec3d.func_186678_a(16.0D)))
        {
            LOGGER.debug("Skipping forward past empty chunk at " + vec3d1);
        }

        LOGGER.debug("Found chunk at " + vec3d1);
        Chunk chunk = func_184301_a(this.worldObj, vec3d1);
        this.exitPortal = func_184307_a(chunk);

        if (this.exitPortal == null)
        {
            this.exitPortal = new BlockPos(vec3d1.xCoord + 0.5D, 75.0D, vec3d1.zCoord + 0.5D);
            LOGGER.debug("Failed to find suitable block, settling on " + this.exitPortal);
            (new WorldGenEndIsland()).generate(this.worldObj, new Random(this.exitPortal.toLong()), this.exitPortal);
        }
        else
        {
            LOGGER.debug("Found block at " + this.exitPortal);
        }

        this.exitPortal = func_184308_a(this.worldObj, this.exitPortal, 16, true);
        LOGGER.debug("Creating portal at " + this.exitPortal);
        this.exitPortal = this.exitPortal.up(10);
        this.func_184312_b(this.exitPortal);
        this.markDirty();
    }

    private static BlockPos func_184308_a(World p_184308_0_, BlockPos p_184308_1_, int p_184308_2_, boolean p_184308_3_)
    {
        BlockPos blockpos = null;

        for (int i = -p_184308_2_; i <= p_184308_2_; ++i)
        {
            for (int j = -p_184308_2_; j <= p_184308_2_; ++j)
            {
                if (i != 0 || j != 0 || p_184308_3_)
                {
                    for (int k = 255; k > (blockpos == null ? 0 : blockpos.getY()); --k)
                    {
                        BlockPos blockpos1 = new BlockPos(p_184308_1_.getX() + i, k, p_184308_1_.getZ() + j);
                        IBlockState iblockstate = p_184308_0_.getBlockState(blockpos1);

                        if (iblockstate.isBlockNormalCube() && (p_184308_3_ || iblockstate.getBlock() != Blocks.bedrock))
                        {
                            blockpos = blockpos1;
                            break;
                        }
                    }
                }
            }
        }

        return blockpos == null ? p_184308_1_ : blockpos;
    }

    private static Chunk func_184301_a(World p_184301_0_, Vec3d p_184301_1_)
    {
        return p_184301_0_.getChunkFromChunkCoords(MathHelper.floor_double(p_184301_1_.xCoord / 16.0D), MathHelper.floor_double(p_184301_1_.zCoord / 16.0D));
    }

    private static BlockPos func_184307_a(Chunk p_184307_0_)
    {
        BlockPos blockpos = new BlockPos(p_184307_0_.xPosition * 16, 30, p_184307_0_.zPosition * 16);
        int i = p_184307_0_.getTopFilledSegment() + 16 - 1;
        BlockPos blockpos1 = new BlockPos(p_184307_0_.xPosition * 16 + 16 - 1, i, p_184307_0_.zPosition * 16 + 16 - 1);
        BlockPos blockpos2 = null;
        double d0 = 0.0D;

        for (BlockPos blockpos3 : BlockPos.getAllInBox(blockpos, blockpos1))
        {
            IBlockState iblockstate = p_184307_0_.getBlockState(blockpos3);

            if (iblockstate.getBlock() == Blocks.end_stone && !p_184307_0_.getBlockState(blockpos3.up(1)).isBlockNormalCube() && !p_184307_0_.getBlockState(blockpos3.up(2)).isBlockNormalCube())
            {
                double d1 = blockpos3.distanceSqToCenter(0.0D, 0.0D, 0.0D);

                if (blockpos2 == null || d1 < d0)
                {
                    blockpos2 = blockpos3;
                    d0 = d1;
                }
            }
        }

        return blockpos2;
    }

    private void func_184312_b(BlockPos p_184312_1_)
    {
        (new WorldGenEndGateway()).generate(this.worldObj, new Random(), p_184312_1_);
        TileEntity tileentity = this.worldObj.getTileEntity(p_184312_1_);

        if (tileentity instanceof TileEntityEndGateway)
        {
            TileEntityEndGateway tileentityendgateway = (TileEntityEndGateway)tileentity;
            tileentityendgateway.exitPortal = new BlockPos(this.getPos());
            tileentityendgateway.markDirty();
        }
        else
        {
            LOGGER.warn("Couldn\'t save exit portal at " + p_184312_1_);
        }
    }

    public boolean func_184313_a(EnumFacing p_184313_1_)
    {
        return this.getBlockType().getDefaultState().shouldSideBeRendered(this.worldObj, this.getPos(), p_184313_1_);
    }

    public int func_184304_i()
    {
        int i = 0;

        for (EnumFacing enumfacing : EnumFacing.values())
        {
            i += this.func_184313_a(enumfacing) ? 1 : 0;
        }

        return i;
    }
}
