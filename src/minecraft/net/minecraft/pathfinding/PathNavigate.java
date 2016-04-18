package net.minecraft.pathfinding;

import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.ChunkCache;
import net.minecraft.world.World;

public abstract class PathNavigate
{
    private static int field_188556_f = 20;
    protected EntityLiving theEntity;
    protected World worldObj;

    /** The PathEntity being followed. */
    protected PathEntity currentPath;
    protected double speed;

    /**
     * The number of blocks (extra) +/- in each axis that get pulled out as cache for the pathfinder's search space
     */
    private final IAttributeInstance pathSearchRange;

    /** Time, in number of ticks, following the current path */
    private int totalTicks;

    /**
     * The time when the last position check was done (to detect successful movement)
     */
    private int ticksAtLastPos;

    /**
     * Coordinates of the entity's position last time a check was done (part of monitoring getting 'stuck')
     */
    private Vec3d lastPosCheck = Vec3d.ZERO;
    private Vec3d field_188557_k = Vec3d.ZERO;
    private long field_188558_l = 0L;
    private long field_188559_m = 0L;
    private double field_188560_n;
    private float field_188561_o = 0.5F;
    private boolean field_188562_p;
    private long field_188563_q;
    protected NodeProcessor nodeProcessor;
    private BlockPos field_188564_r;
    private final PathFinder pathFinder;

    public PathNavigate(EntityLiving entitylivingIn, World worldIn)
    {
        this.theEntity = entitylivingIn;
        this.worldObj = worldIn;
        this.pathSearchRange = entitylivingIn.getEntityAttribute(SharedMonsterAttributes.FOLLOW_RANGE);
        this.pathFinder = this.getPathFinder();
        this.worldObj.func_184140_C().func_184379_a(this);
    }

    protected abstract PathFinder getPathFinder();

    /**
     * Sets the speed
     */
    public void setSpeed(double speedIn)
    {
        this.speed = speedIn;
    }

    /**
     * Gets the maximum distance that the path finding will search in.
     */
    public float getPathSearchRange()
    {
        return (float)this.pathSearchRange.getAttributeValue();
    }

    public boolean func_188553_i()
    {
        return this.field_188562_p;
    }

    public void func_188554_j()
    {
        if (this.worldObj.getTotalWorldTime() - this.field_188563_q > (long)field_188556_f)
        {
            if (this.field_188564_r != null)
            {
                this.currentPath = null;
                this.currentPath = this.getPathToPos(this.field_188564_r);
                this.field_188563_q = this.worldObj.getTotalWorldTime();
                this.field_188562_p = false;
            }
        }
        else
        {
            this.field_188562_p = true;
        }
    }

    /**
     * Returns the path to the given coordinates. Args : x, y, z
     */
    public final PathEntity getPathToXYZ(double x, double y, double z)
    {
        return this.getPathToPos(new BlockPos(MathHelper.floor_double(x), (int)y, MathHelper.floor_double(z)));
    }

    /**
     * Returns path to given BlockPos
     */
    public PathEntity getPathToPos(BlockPos pos)
    {
        if (!this.canNavigate())
        {
            return null;
        }
        else if (this.currentPath != null && !this.currentPath.isFinished() && pos.equals(this.field_188564_r))
        {
            return this.currentPath;
        }
        else
        {
            this.field_188564_r = pos;
            float f = this.getPathSearchRange();
            this.worldObj.theProfiler.startSection("pathfind");
            BlockPos blockpos = new BlockPos(this.theEntity);
            int i = (int)(f + 8.0F);
            ChunkCache chunkcache = new ChunkCache(this.worldObj, blockpos.add(-i, -i, -i), blockpos.add(i, i, i), 0);
            PathEntity pathentity = this.pathFinder.func_186336_a(chunkcache, this.theEntity, this.field_188564_r, f);
            this.worldObj.theProfiler.endSection();
            return pathentity;
        }
    }

    /**
     * Returns the path to the given EntityLiving. Args : entity
     */
    public PathEntity getPathToEntityLiving(Entity entityIn)
    {
        if (!this.canNavigate())
        {
            return null;
        }
        else
        {
            BlockPos blockpos = new BlockPos(entityIn);

            if (this.currentPath != null && !this.currentPath.isFinished() && blockpos.equals(this.field_188564_r))
            {
                return this.currentPath;
            }
            else
            {
                this.field_188564_r = blockpos;
                float f = this.getPathSearchRange();
                this.worldObj.theProfiler.startSection("pathfind");
                BlockPos blockpos1 = (new BlockPos(this.theEntity)).up();
                int i = (int)(f + 16.0F);
                ChunkCache chunkcache = new ChunkCache(this.worldObj, blockpos1.add(-i, -i, -i), blockpos1.add(i, i, i), 0);
                PathEntity pathentity = this.pathFinder.func_186333_a(chunkcache, this.theEntity, entityIn, f);
                this.worldObj.theProfiler.endSection();
                return pathentity;
            }
        }
    }

    /**
     * Try to find and set a path to XYZ. Returns true if successful. Args : x, y, z, speed
     */
    public boolean tryMoveToXYZ(double x, double y, double z, double speedIn)
    {
        PathEntity pathentity = this.getPathToXYZ((double)MathHelper.floor_double(x), (double)((int)y), (double)MathHelper.floor_double(z));
        return this.setPath(pathentity, speedIn);
    }

    /**
     * Try to find and set a path to EntityLiving. Returns true if successful. Args : entity, speed
     */
    public boolean tryMoveToEntityLiving(Entity entityIn, double speedIn)
    {
        PathEntity pathentity = this.getPathToEntityLiving(entityIn);
        return pathentity != null ? this.setPath(pathentity, speedIn) : false;
    }

    /**
     * Sets a new path. If it's diferent from the old path. Checks to adjust path for sun avoiding, and stores start
     * coords. Args : path, speed
     */
    public boolean setPath(PathEntity pathentityIn, double speedIn)
    {
        if (pathentityIn == null)
        {
            this.currentPath = null;
            return false;
        }
        else
        {
            if (!pathentityIn.isSamePath(this.currentPath))
            {
                this.currentPath = pathentityIn;
            }

            this.removeSunnyPath();

            if (this.currentPath.getCurrentPathLength() == 0)
            {
                return false;
            }
            else
            {
                this.speed = speedIn;
                Vec3d vec3d = this.getEntityPosition();
                this.ticksAtLastPos = this.totalTicks;
                this.lastPosCheck = vec3d;
                return true;
            }
        }
    }

    /**
     * gets the actively used PathEntity
     */
    public PathEntity getPath()
    {
        return this.currentPath;
    }

    public void onUpdateNavigation()
    {
        ++this.totalTicks;

        if (this.field_188562_p)
        {
            this.func_188554_j();
        }

        if (!this.noPath())
        {
            if (this.canNavigate())
            {
                this.pathFollow();
            }
            else if (this.currentPath != null && this.currentPath.getCurrentPathIndex() < this.currentPath.getCurrentPathLength())
            {
                Vec3d vec3d = this.getEntityPosition();
                Vec3d vec3d1 = this.currentPath.getVectorFromIndex(this.theEntity, this.currentPath.getCurrentPathIndex());

                if (vec3d.yCoord > vec3d1.yCoord && !this.theEntity.onGround && MathHelper.floor_double(vec3d.xCoord) == MathHelper.floor_double(vec3d1.xCoord) && MathHelper.floor_double(vec3d.zCoord) == MathHelper.floor_double(vec3d1.zCoord))
                {
                    this.currentPath.setCurrentPathIndex(this.currentPath.getCurrentPathIndex() + 1);
                }
            }

            if (!this.noPath())
            {
                Vec3d vec3d2 = this.currentPath.getPosition(this.theEntity);

                if (vec3d2 != null)
                {
                    BlockPos blockpos = (new BlockPos(vec3d2)).down();
                    AxisAlignedBB axisalignedbb = this.worldObj.getBlockState(blockpos).func_185900_c(this.worldObj, blockpos);
                    vec3d2 = vec3d2.subtract(0.0D, 1.0D - axisalignedbb.maxY, 0.0D);
                    this.theEntity.getMoveHelper().setMoveTo(vec3d2.xCoord, vec3d2.yCoord, vec3d2.zCoord, this.speed);
                }
            }
        }
    }

    protected void pathFollow()
    {
        Vec3d vec3d = this.getEntityPosition();
        int i = this.currentPath.getCurrentPathLength();

        for (int j = this.currentPath.getCurrentPathIndex(); j < this.currentPath.getCurrentPathLength(); ++j)
        {
            if ((double)this.currentPath.getPathPointFromIndex(j).yCoord != Math.floor(vec3d.yCoord))
            {
                i = j;
                break;
            }
        }

        this.field_188561_o = this.theEntity.width > 0.75F ? this.theEntity.width / 2.0F : 0.75F - this.theEntity.width / 2.0F;
        Vec3d vec3d1 = this.currentPath.func_186310_f();

        if (MathHelper.abs((float)(this.theEntity.posX - (vec3d1.xCoord + 0.5D))) < this.field_188561_o && MathHelper.abs((float)(this.theEntity.posZ - (vec3d1.zCoord + 0.5D))) < this.field_188561_o)
        {
            this.currentPath.setCurrentPathIndex(this.currentPath.getCurrentPathIndex() + 1);
        }

        int k = MathHelper.ceiling_float_int(this.theEntity.width);
        int l = (int)this.theEntity.height + 1;
        int i1 = k;

        for (int j1 = i - 1; j1 >= this.currentPath.getCurrentPathIndex(); --j1)
        {
            if (this.isDirectPathBetweenPoints(vec3d, this.currentPath.getVectorFromIndex(this.theEntity, j1), k, l, i1))
            {
                this.currentPath.setCurrentPathIndex(j1);
                break;
            }
        }

        this.checkForStuck(vec3d);
    }

    /**
     * Checks if entity haven't been moved when last checked and if so, clears current {@link
     * net.minecraft.pathfinding.PathEntity}
     */
    protected void checkForStuck(Vec3d positionVec3)
    {
        if (this.totalTicks - this.ticksAtLastPos > 100)
        {
            if (positionVec3.squareDistanceTo(this.lastPosCheck) < 2.25D)
            {
                this.clearPathEntity();
            }

            this.ticksAtLastPos = this.totalTicks;
            this.lastPosCheck = positionVec3;
        }

        if (this.currentPath != null && !this.currentPath.isFinished())
        {
            Vec3d vec3d = this.currentPath.func_186310_f();

            if (!vec3d.equals(this.field_188557_k))
            {
                this.field_188557_k = vec3d;
                double d0 = positionVec3.distanceTo(this.field_188557_k);
                this.field_188560_n = this.theEntity.getAIMoveSpeed() > 0.0F ? d0 / (double)this.theEntity.getAIMoveSpeed() * 1000.0D : 0.0D;
            }
            else
            {
                this.field_188558_l += System.currentTimeMillis() - this.field_188559_m;
            }

            if (this.field_188560_n > 0.0D && (double)this.field_188558_l > this.field_188560_n * 3.0D)
            {
                this.field_188557_k = Vec3d.ZERO;
                this.field_188558_l = 0L;
                this.field_188560_n = 0.0D;
                this.clearPathEntity();
            }

            this.field_188559_m = System.currentTimeMillis();
        }
    }

    /**
     * If null path or reached the end
     */
    public boolean noPath()
    {
        return this.currentPath == null || this.currentPath.isFinished();
    }

    /**
     * sets active PathEntity to null
     */
    public void clearPathEntity()
    {
        this.currentPath = null;
    }

    protected abstract Vec3d getEntityPosition();

    /**
     * If on ground or swimming and can swim
     */
    protected abstract boolean canNavigate();

    /**
     * Returns true if the entity is in water or lava, false otherwise
     */
    protected boolean isInLiquid()
    {
        return this.theEntity.isInWater() || this.theEntity.isInLava();
    }

    /**
     * Trims path data from the end to the first sun covered block
     */
    protected void removeSunnyPath()
    {
    }

    /**
     * Returns true when an entity of specified size could safely walk in a straight line between the two points. Args:
     * pos1, pos2, entityXSize, entityYSize, entityZSize
     */
    protected abstract boolean isDirectPathBetweenPoints(Vec3d posVec31, Vec3d posVec32, int sizeX, int sizeY, int sizeZ);

    public boolean func_188555_b(BlockPos p_188555_1_)
    {
        return this.worldObj.getBlockState(p_188555_1_.down()).isFullBlock();
    }
}
