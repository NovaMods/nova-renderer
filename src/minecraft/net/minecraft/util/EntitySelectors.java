package net.minecraft.util;

import com.google.common.base.Predicate;
import com.google.common.base.Predicates;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.item.EntityArmorStand;
import net.minecraft.entity.monster.EntityShulker;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.inventory.IInventory;
import net.minecraft.item.ItemStack;
import net.minecraft.scoreboard.Team;

public final class EntitySelectors
{
    public static final Predicate<Entity> selectAnything = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_.isEntityAlive();
        }
    };
    public static final Predicate<Entity> IS_STANDALONE = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_.isEntityAlive() && !p_apply_1_.isBeingRidden() && !p_apply_1_.isRiding();
        }
    };
    public static final Predicate<Entity> selectInventories = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_ instanceof IInventory && p_apply_1_.isEntityAlive();
        }
    };
    public static final Predicate<Entity> field_188444_d = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return !(p_apply_1_ instanceof EntityPlayer) || !((EntityPlayer)p_apply_1_).isSpectator() && !((EntityPlayer)p_apply_1_).isCreative();
        }
    };
    public static final Predicate<Entity> NOT_SPECTATING = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return !(p_apply_1_ instanceof EntityPlayer) || !((EntityPlayer)p_apply_1_).isSpectator();
        }
    };
    public static final Predicate<Entity> field_188445_f = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_ instanceof EntityShulker && p_apply_1_.isEntityAlive();
        }
    };

    public static <T extends Entity> Predicate<T> func_188443_a(final double p_188443_0_, final double p_188443_2_, final double p_188443_4_, double p_188443_6_)
    {
        final double d0 = p_188443_6_ * p_188443_6_;
        return new Predicate<T>()
        {
            public boolean apply(T p_apply_1_)
            {
                return p_apply_1_ != null && p_apply_1_.getDistanceSq(p_188443_0_, p_188443_2_, p_188443_4_) <= d0;
            }
        };
    }

    public static <T extends Entity> Predicate<T> func_188442_a(final Entity p_188442_0_)
    {
        final Team team = p_188442_0_.getTeam();
        final Team.CollisionRule team$collisionrule = team == null ? Team.CollisionRule.ALWAYS : team.getCollisionRule();
        Predicate<?> ret = team$collisionrule == Team.CollisionRule.NEVER ? Predicates.alwaysFalse() : Predicates.and(NOT_SPECTATING, new Predicate<Entity>()
        {
            public boolean apply(Entity p_apply_1_)
            {
                if (!p_apply_1_.canBePushed())
                {
                    return false;
                }
                else if (!p_188442_0_.worldObj.isRemote || p_apply_1_ instanceof EntityPlayer && ((EntityPlayer)p_apply_1_).isUser())
                {
                    Team team1 = p_apply_1_.getTeam();
                    Team.CollisionRule team$collisionrule1 = team1 == null ? Team.CollisionRule.ALWAYS : team1.getCollisionRule();

                    if (team$collisionrule1 == Team.CollisionRule.NEVER)
                    {
                        return false;
                    }
                    else
                    {
                        boolean flag = team != null && team.isSameTeam(team1);
                        return (team$collisionrule == Team.CollisionRule.HIDE_FOR_OWN_TEAM || team$collisionrule1 == Team.CollisionRule.HIDE_FOR_OWN_TEAM) && flag ? false : team$collisionrule != Team.CollisionRule.HIDE_FOR_OTHER_TEAMS && team$collisionrule1 != Team.CollisionRule.HIDE_FOR_OTHER_TEAMS || flag;
                    }
                }
                else
                {
                    return false;
                }
            }
        });
        return (Predicate<T>)ret;
    }

    public static class ArmoredMob implements Predicate<Entity>
    {
        private final ItemStack armor;

        public ArmoredMob(ItemStack armor)
        {
            this.armor = armor;
        }

        public boolean apply(Entity p_apply_1_)
        {
            if (!p_apply_1_.isEntityAlive())
            {
                return false;
            }
            else if (!(p_apply_1_ instanceof EntityLivingBase))
            {
                return false;
            }
            else
            {
                EntityLivingBase entitylivingbase = (EntityLivingBase)p_apply_1_;
                return entitylivingbase.getItemStackFromSlot(EntityLiving.func_184640_d(this.armor)) != null ? false : (entitylivingbase instanceof EntityLiving ? ((EntityLiving)entitylivingbase).canPickUpLoot() : (entitylivingbase instanceof EntityArmorStand ? true : entitylivingbase instanceof EntityPlayer));
            }
        }
    }
}
