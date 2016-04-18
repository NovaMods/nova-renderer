package net.minecraft.pathfinding;

import java.util.HashSet;
import java.util.Set;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;

public class PathFinder
{
    /** The path being generated */
    private final Path path = new Path();
    private final Set<PathPoint> field_186337_b = new HashSet();

    /** Selection of path points to add to the path */
    private final PathPoint[] pathOptions = new PathPoint[32];
    private final NodeProcessor nodeProcessor;

    public PathFinder(NodeProcessor p_i46652_1_)
    {
        this.nodeProcessor = p_i46652_1_;
    }

    public PathEntity func_186333_a(IBlockAccess p_186333_1_, EntityLiving p_186333_2_, Entity p_186333_3_, float p_186333_4_)
    {
        return this.func_186334_a(p_186333_1_, p_186333_2_, p_186333_3_.posX, p_186333_3_.getEntityBoundingBox().minY, p_186333_3_.posZ, p_186333_4_);
    }

    public PathEntity func_186336_a(IBlockAccess p_186336_1_, EntityLiving p_186336_2_, BlockPos p_186336_3_, float p_186336_4_)
    {
        return this.func_186334_a(p_186336_1_, p_186336_2_, (double)((float)p_186336_3_.getX() + 0.5F), (double)((float)p_186336_3_.getY() + 0.5F), (double)((float)p_186336_3_.getZ() + 0.5F), p_186336_4_);
    }

    private PathEntity func_186334_a(IBlockAccess p_186334_1_, EntityLiving p_186334_2_, double p_186334_3_, double p_186334_5_, double p_186334_7_, float p_186334_9_)
    {
        this.path.clearPath();
        this.nodeProcessor.func_186315_a(p_186334_1_, p_186334_2_);
        PathPoint pathpoint = this.nodeProcessor.func_186318_b();
        PathPoint pathpoint1 = this.nodeProcessor.func_186325_a(p_186334_3_, p_186334_5_, p_186334_7_);
        PathEntity pathentity = this.func_186335_a(pathpoint, pathpoint1, p_186334_9_);
        this.nodeProcessor.postProcess();
        return pathentity;
    }

    private PathEntity func_186335_a(PathPoint p_186335_1_, PathPoint p_186335_2_, float p_186335_3_)
    {
        p_186335_1_.totalPathDistance = 0.0F;
        p_186335_1_.distanceToNext = p_186335_1_.func_186281_c(p_186335_2_);
        p_186335_1_.distanceToTarget = p_186335_1_.distanceToNext;
        this.path.clearPath();
        this.field_186337_b.clear();
        this.path.addPoint(p_186335_1_);
        PathPoint pathpoint = p_186335_1_;
        int i = 0;

        while (!this.path.isPathEmpty())
        {
            ++i;

            if (i >= 2000)
            {
                break;
            }

            PathPoint pathpoint1 = this.path.dequeue();

            if (pathpoint1.equals(p_186335_2_))
            {
                pathpoint = p_186335_2_;
                break;
            }

            if (pathpoint1.func_186281_c(p_186335_2_) < pathpoint.func_186281_c(p_186335_2_))
            {
                pathpoint = pathpoint1;
            }

            pathpoint1.visited = true;
            int j = this.nodeProcessor.func_186320_a(this.pathOptions, pathpoint1, p_186335_2_, p_186335_3_);

            for (int k = 0; k < j; ++k)
            {
                PathPoint pathpoint2 = this.pathOptions[k];
                float f = pathpoint1.func_186281_c(pathpoint2);
                pathpoint2.field_186284_j = pathpoint1.field_186284_j + f;
                pathpoint2.field_186285_k = f + pathpoint2.field_186286_l;
                float f1 = pathpoint1.totalPathDistance + pathpoint2.field_186285_k;

                if (pathpoint2.field_186284_j < p_186335_3_ && (!pathpoint2.isAssigned() || f1 < pathpoint2.totalPathDistance))
                {
                    pathpoint2.previous = pathpoint1;
                    pathpoint2.totalPathDistance = f1;
                    pathpoint2.distanceToNext = pathpoint2.func_186281_c(p_186335_2_) + pathpoint2.field_186286_l;

                    if (pathpoint2.isAssigned())
                    {
                        this.path.changeDistance(pathpoint2, pathpoint2.totalPathDistance + pathpoint2.distanceToNext);
                    }
                    else
                    {
                        pathpoint2.distanceToTarget = pathpoint2.totalPathDistance + pathpoint2.distanceToNext;
                        this.path.addPoint(pathpoint2);
                    }
                }
            }
        }

        if (pathpoint == p_186335_1_)
        {
            return null;
        }
        else
        {
            PathEntity pathentity = this.createEntityPath(p_186335_1_, pathpoint);
            return pathentity;
        }
    }

    /**
     * Returns a new PathEntity for a given start and end point
     */
    private PathEntity createEntityPath(PathPoint start, PathPoint end)
    {
        int i = 1;

        for (PathPoint pathpoint = end; pathpoint.previous != null; pathpoint = pathpoint.previous)
        {
            ++i;
        }

        PathPoint[] apathpoint = new PathPoint[i];
        PathPoint pathpoint1 = end;
        --i;

        for (apathpoint[i] = end; pathpoint1.previous != null; apathpoint[i] = pathpoint1)
        {
            pathpoint1 = pathpoint1.previous;
            --i;
        }

        return new PathEntity(apathpoint);
    }
}
