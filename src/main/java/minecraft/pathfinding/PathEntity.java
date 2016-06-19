package net.minecraft.pathfinding;

import net.minecraft.entity.Entity;
import net.minecraft.network.PacketBuffer;
import net.minecraft.util.math.Vec3d;

public class PathEntity
{
    /** The actual points in the path */
    private final PathPoint[] points;
    private PathPoint[] field_186312_b = new PathPoint[0];
    private PathPoint[] field_186313_c = new PathPoint[0];
    private PathPoint field_186314_d;

    /** PathEntity Array Index the Entity is currently targeting */
    private int currentPathIndex;

    /** The total length of the path */
    private int pathLength;

    public PathEntity(PathPoint[] pathpoints)
    {
        this.points = pathpoints;
        this.pathLength = pathpoints.length;
    }

    /**
     * Directs this path to the next point in its array
     */
    public void incrementPathIndex()
    {
        ++this.currentPathIndex;
    }

    /**
     * Returns true if this path has reached the end
     */
    public boolean isFinished()
    {
        return this.currentPathIndex >= this.pathLength;
    }

    /**
     * returns the last PathPoint of the Array
     */
    public PathPoint getFinalPathPoint()
    {
        return this.pathLength > 0 ? this.points[this.pathLength - 1] : null;
    }

    /**
     * return the PathPoint located at the specified PathIndex, usually the current one
     */
    public PathPoint getPathPointFromIndex(int index)
    {
        return this.points[index];
    }

    public void func_186309_a(int p_186309_1_, PathPoint p_186309_2_)
    {
        this.points[p_186309_1_] = p_186309_2_;
    }

    public int getCurrentPathLength()
    {
        return this.pathLength;
    }

    public void setCurrentPathLength(int length)
    {
        this.pathLength = length;
    }

    public int getCurrentPathIndex()
    {
        return this.currentPathIndex;
    }

    public void setCurrentPathIndex(int currentPathIndexIn)
    {
        this.currentPathIndex = currentPathIndexIn;
    }

    /**
     * Gets the vector of the PathPoint associated with the given index.
     */
    public Vec3d getVectorFromIndex(Entity entityIn, int index)
    {
        double d0 = (double)this.points[index].xCoord + (double)((int)(entityIn.width + 1.0F)) * 0.5D;
        double d1 = (double)this.points[index].yCoord;
        double d2 = (double)this.points[index].zCoord + (double)((int)(entityIn.width + 1.0F)) * 0.5D;
        return new Vec3d(d0, d1, d2);
    }

    /**
     * returns the current PathEntity target node as Vec3D
     */
    public Vec3d getPosition(Entity entityIn)
    {
        return this.getVectorFromIndex(entityIn, this.currentPathIndex);
    }

    public Vec3d func_186310_f()
    {
        PathPoint pathpoint = this.points[this.currentPathIndex];
        return new Vec3d((double)pathpoint.xCoord, (double)pathpoint.yCoord, (double)pathpoint.zCoord);
    }

    /**
     * Returns true if the EntityPath are the same. Non instance related equals.
     */
    public boolean isSamePath(PathEntity pathentityIn)
    {
        if (pathentityIn == null)
        {
            return false;
        }
        else if (pathentityIn.points.length != this.points.length)
        {
            return false;
        }
        else
        {
            for (int i = 0; i < this.points.length; ++i)
            {
                if (this.points[i].xCoord != pathentityIn.points[i].xCoord || this.points[i].yCoord != pathentityIn.points[i].yCoord || this.points[i].zCoord != pathentityIn.points[i].zCoord)
                {
                    return false;
                }
            }

            return true;
        }
    }

    /**
     * Returns true if the final PathPoint in the PathEntity is equal to Vec3D coords.
     */
    public boolean isDestinationSame(Vec3d vec)
    {
        PathPoint pathpoint = this.getFinalPathPoint();
        return pathpoint == null ? false : pathpoint.xCoord == (int)vec.xCoord && pathpoint.zCoord == (int)vec.zCoord;
    }

    public static PathEntity func_186311_b(PacketBuffer p_186311_0_)
    {
        int i = p_186311_0_.readInt();
        PathPoint pathpoint = PathPoint.func_186282_b(p_186311_0_);
        PathPoint[] apathpoint = new PathPoint[p_186311_0_.readInt()];

        for (int j = 0; j < apathpoint.length; ++j)
        {
            apathpoint[j] = PathPoint.func_186282_b(p_186311_0_);
        }

        PathPoint[] apathpoint1 = new PathPoint[p_186311_0_.readInt()];

        for (int k = 0; k < apathpoint1.length; ++k)
        {
            apathpoint1[k] = PathPoint.func_186282_b(p_186311_0_);
        }

        PathPoint[] apathpoint2 = new PathPoint[p_186311_0_.readInt()];

        for (int l = 0; l < apathpoint2.length; ++l)
        {
            apathpoint2[l] = PathPoint.func_186282_b(p_186311_0_);
        }

        PathEntity pathentity = new PathEntity(apathpoint);
        pathentity.field_186312_b = apathpoint1;
        pathentity.field_186313_c = apathpoint2;
        pathentity.field_186314_d = pathpoint;
        pathentity.currentPathIndex = i;
        return pathentity;
    }
}
