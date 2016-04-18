package net.minecraft.entity.projectile;

import java.util.List;
import net.minecraft.entity.EntityAreaEffectCloud;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.MobEffects;
import net.minecraft.potion.PotionEffect;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.world.World;

public class EntityDragonFireball extends EntityFireball
{
    public EntityDragonFireball(World worldIn)
    {
        super(worldIn);
        this.setSize(0.3125F, 0.3125F);
    }

    public EntityDragonFireball(World worldIn, double x, double y, double z, double accelX, double accelY, double accelZ)
    {
        super(worldIn, x, y, z, accelX, accelY, accelZ);
        this.setSize(0.3125F, 0.3125F);
    }

    public EntityDragonFireball(World worldIn, EntityLivingBase shooter, double accelX, double accelY, double accelZ)
    {
        super(worldIn, shooter, accelX, accelY, accelZ);
        this.setSize(0.3125F, 0.3125F);
    }

    /**
     * Called when this EntityFireball hits a block or entity.
     */
    protected void onImpact(RayTraceResult movingObject)
    {
        if (!this.worldObj.isRemote)
        {
            List<EntityLivingBase> list = this.worldObj.<EntityLivingBase>getEntitiesWithinAABB(EntityLivingBase.class, this.getEntityBoundingBox().expand(4.0D, 2.0D, 4.0D));
            EntityAreaEffectCloud entityareaeffectcloud = new EntityAreaEffectCloud(this.worldObj, this.posX, this.posY, this.posZ);
            entityareaeffectcloud.func_184481_a(this.shootingEntity);
            entityareaeffectcloud.func_184491_a(EnumParticleTypes.DRAGON_BREATH);
            entityareaeffectcloud.setRadius(3.0F);
            entityareaeffectcloud.func_184486_b(2400);
            entityareaeffectcloud.func_184487_c((7.0F - entityareaeffectcloud.getRadius()) / (float)entityareaeffectcloud.func_184489_o());
            entityareaeffectcloud.func_184496_a(new PotionEffect(MobEffects.harm, 1, 1));

            if (!list.isEmpty())
            {
                for (EntityLivingBase entitylivingbase : list)
                {
                    double d0 = this.getDistanceSqToEntity(entitylivingbase);

                    if (d0 < 16.0D)
                    {
                        entityareaeffectcloud.setPosition(entitylivingbase.posX, entitylivingbase.posY, entitylivingbase.posZ);
                        break;
                    }
                }
            }

            this.worldObj.playAuxSFX(2006, new BlockPos(this.posX, this.posY, this.posZ), 0);
            this.worldObj.spawnEntityInWorld(entityareaeffectcloud);
            this.setDead();
        }
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return false;
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        return false;
    }

    protected EnumParticleTypes func_184563_j()
    {
        return EnumParticleTypes.DRAGON_BREATH;
    }

    protected boolean func_184564_k()
    {
        return false;
    }
}
