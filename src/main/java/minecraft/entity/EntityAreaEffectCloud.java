package net.minecraft.entity;

import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.Map.Entry;
import net.minecraft.block.material.EnumPushReaction;
import net.minecraft.init.PotionTypes;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.potion.PotionEffect;
import net.minecraft.potion.PotionType;
import net.minecraft.potion.PotionUtils;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;

public class EntityAreaEffectCloud extends Entity
{
    private static final DataParameter<Float> RADIUS = EntityDataManager.<Float>createKey(EntityAreaEffectCloud.class, DataSerializers.FLOAT);
    private static final DataParameter<Integer> field_184499_b = EntityDataManager.<Integer>createKey(EntityAreaEffectCloud.class, DataSerializers.VARINT);
    private static final DataParameter<Boolean> field_184500_c = EntityDataManager.<Boolean>createKey(EntityAreaEffectCloud.class, DataSerializers.BOOLEAN);
    private static final DataParameter<Integer> PARTICLE = EntityDataManager.<Integer>createKey(EntityAreaEffectCloud.class, DataSerializers.VARINT);
    private PotionType field_184502_e;
    private final List<PotionEffect> field_184503_f;
    private final Map<Entity, Integer> field_184504_g;
    private int duration;
    private int waitTime;
    private int reapplicationDelay;
    private boolean field_184508_au;
    private int durationOnUse;
    private float radiusOnUse;
    private float radiusPerTick;
    private EntityLivingBase field_184512_ay;
    private UUID ownerUniqueId;

    public EntityAreaEffectCloud(World worldIn)
    {
        super(worldIn);
        this.field_184502_e = PotionTypes.empty;
        this.field_184503_f = Lists.<PotionEffect>newArrayList();
        this.field_184504_g = Maps.<Entity, Integer>newHashMap();
        this.duration = 600;
        this.waitTime = 20;
        this.reapplicationDelay = 20;
        this.noClip = true;
        this.isImmuneToFire = true;
        this.setRadius(3.0F);
    }

    public EntityAreaEffectCloud(World worldIn, double x, double y, double z)
    {
        this(worldIn);
        this.setPosition(x, y, z);
    }

    protected void entityInit()
    {
        this.getDataManager().register(field_184499_b, Integer.valueOf(0));
        this.getDataManager().register(RADIUS, Float.valueOf(0.5F));
        this.getDataManager().register(field_184500_c, Boolean.valueOf(false));
        this.getDataManager().register(PARTICLE, Integer.valueOf(EnumParticleTypes.SPELL_MOB.getParticleID()));
    }

    public void setRadius(float p_184483_1_)
    {
        double d0 = this.posX;
        double d1 = this.posY;
        double d2 = this.posZ;
        this.setSize(p_184483_1_ * 2.0F, 0.5F);
        this.setPosition(d0, d1, d2);

        if (!this.worldObj.isRemote)
        {
            this.getDataManager().set(RADIUS, Float.valueOf(p_184483_1_));
        }
    }

    public float getRadius()
    {
        return ((Float)this.getDataManager().get(RADIUS)).floatValue();
    }

    public void func_184484_a(PotionType p_184484_1_)
    {
        this.field_184502_e = p_184484_1_;

        if (!this.field_184508_au)
        {
            if (p_184484_1_ == PotionTypes.empty && this.field_184503_f.isEmpty())
            {
                this.getDataManager().set(field_184499_b, Integer.valueOf(0));
            }
            else
            {
                this.getDataManager().set(field_184499_b, Integer.valueOf(PotionUtils.func_185181_a(PotionUtils.func_185186_a(p_184484_1_, this.field_184503_f))));
            }
        }
    }

    public void func_184496_a(PotionEffect p_184496_1_)
    {
        this.field_184503_f.add(p_184496_1_);

        if (!this.field_184508_au)
        {
            this.getDataManager().set(field_184499_b, Integer.valueOf(PotionUtils.func_185181_a(PotionUtils.func_185186_a(this.field_184502_e, this.field_184503_f))));
        }
    }

    public int func_184492_k()
    {
        return ((Integer)this.getDataManager().get(field_184499_b)).intValue();
    }

    public void func_184482_a(int p_184482_1_)
    {
        this.field_184508_au = true;
        this.getDataManager().set(field_184499_b, Integer.valueOf(p_184482_1_));
    }

    public EnumParticleTypes func_184493_l()
    {
        return EnumParticleTypes.getParticleFromId(((Integer)this.getDataManager().get(PARTICLE)).intValue());
    }

    public void func_184491_a(EnumParticleTypes p_184491_1_)
    {
        this.getDataManager().set(PARTICLE, Integer.valueOf(p_184491_1_.getParticleID()));
    }

    protected void func_184488_a(boolean p_184488_1_)
    {
        this.getDataManager().set(field_184500_c, Boolean.valueOf(p_184488_1_));
    }

    public boolean func_184497_n()
    {
        return ((Boolean)this.getDataManager().get(field_184500_c)).booleanValue();
    }

    public int func_184489_o()
    {
        return this.duration;
    }

    public void func_184486_b(int p_184486_1_)
    {
        this.duration = p_184486_1_;
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
        boolean flag = this.func_184497_n();
        float f = this.getRadius();

        if (this.worldObj.isRemote)
        {
            EnumParticleTypes enumparticletypes = this.func_184493_l();

            if (flag)
            {
                if (this.rand.nextBoolean())
                {
                    int[] aint = new int[enumparticletypes.getArgumentCount()];

                    for (int i = 0; i < 2; ++i)
                    {
                        float f1 = this.rand.nextFloat() * ((float)Math.PI * 2F);
                        float f2 = MathHelper.sqrt_float(this.rand.nextFloat()) * 0.2F;
                        float f3 = MathHelper.cos(f1) * f2;
                        float f4 = MathHelper.sin(f1) * f2;

                        if (enumparticletypes == EnumParticleTypes.SPELL_MOB)
                        {
                            int j = this.rand.nextBoolean() ? 16777215 : this.func_184492_k();
                            int k = j >> 16 & 255;
                            int l = j >> 8 & 255;
                            int i1 = j & 255;
                            this.worldObj.spawnParticle(EnumParticleTypes.SPELL_MOB, this.posX + (double)f3, this.posY, this.posZ + (double)f4, (double)((float)k / 255.0F), (double)((float)l / 255.0F), (double)((float)i1 / 255.0F), new int[0]);
                        }
                        else
                        {
                            this.worldObj.spawnParticle(enumparticletypes, this.posX + (double)f3, this.posY, this.posZ + (double)f4, 0.0D, 0.0D, 0.0D, aint);
                        }
                    }
                }
            }
            else
            {
                float f5 = (float)Math.PI * f * f;
                int[] aint1 = new int[enumparticletypes.getArgumentCount()];

                for (int k1 = 0; (float)k1 < f5; ++k1)
                {
                    float f6 = this.rand.nextFloat() * ((float)Math.PI * 2F);
                    float f7 = MathHelper.sqrt_float(this.rand.nextFloat()) * f;
                    float f8 = MathHelper.cos(f6) * f7;
                    float f9 = MathHelper.sin(f6) * f7;

                    if (enumparticletypes == EnumParticleTypes.SPELL_MOB)
                    {
                        int l1 = this.func_184492_k();
                        int i2 = l1 >> 16 & 255;
                        int j2 = l1 >> 8 & 255;
                        int j1 = l1 & 255;
                        this.worldObj.spawnParticle(EnumParticleTypes.SPELL_MOB, this.posX + (double)f8, this.posY, this.posZ + (double)f9, (double)((float)i2 / 255.0F), (double)((float)j2 / 255.0F), (double)((float)j1 / 255.0F), new int[0]);
                    }
                    else
                    {
                        this.worldObj.spawnParticle(enumparticletypes, this.posX + (double)f8, this.posY, this.posZ + (double)f9, (0.5D - this.rand.nextDouble()) * 0.15D, 0.009999999776482582D, (0.5D - this.rand.nextDouble()) * 0.15D, aint1);
                    }
                }
            }
        }
        else
        {
            if (this.ticksExisted >= this.waitTime + this.duration)
            {
                this.setDead();
                return;
            }

            boolean flag1 = this.ticksExisted < this.waitTime;

            if (flag != flag1)
            {
                this.func_184488_a(flag1);
            }

            if (flag1)
            {
                return;
            }

            if (this.radiusPerTick != 0.0F)
            {
                f += this.radiusPerTick;

                if (f < 0.5F)
                {
                    this.setDead();
                    return;
                }

                this.setRadius(f);
            }

            if (this.ticksExisted % 5 == 0)
            {
                Iterator<Entry<Entity, Integer>> iterator = this.field_184504_g.entrySet().iterator();

                while (iterator.hasNext())
                {
                    Entry<Entity, Integer> entry = (Entry)iterator.next();

                    if (this.ticksExisted >= ((Integer)entry.getValue()).intValue())
                    {
                        iterator.remove();
                    }
                }

                List<PotionEffect> potions = Lists.newArrayList();

                for (PotionEffect potioneffect1 : this.field_184502_e.getEffects())
                {
                    potions.add(new PotionEffect(potioneffect1.func_188419_a(), potioneffect1.getDuration() / 4, potioneffect1.getAmplifier(), potioneffect1.getIsAmbient(), potioneffect1.func_188418_e()));
                }

                potions.addAll(this.field_184503_f);

                if (potions.isEmpty())
                {
                    this.field_184504_g.clear();
                }
                else
                {
                    List<EntityLivingBase> list = this.worldObj.<EntityLivingBase>getEntitiesWithinAABB(EntityLivingBase.class, this.getEntityBoundingBox());

                    if (!list.isEmpty())
                    {
                        for (EntityLivingBase entitylivingbase : list)
                        {
                            if (!this.field_184504_g.containsKey(entitylivingbase) && entitylivingbase.func_184603_cC())
                            {
                                double d0 = entitylivingbase.posX - this.posX;
                                double d1 = entitylivingbase.posZ - this.posZ;
                                double d2 = d0 * d0 + d1 * d1;

                                if (d2 <= (double)(f * f))
                                {
                                    this.field_184504_g.put(entitylivingbase, Integer.valueOf(this.ticksExisted + this.reapplicationDelay));

                                    for (PotionEffect potioneffect : potions)
                                    {
                                        if (potioneffect.func_188419_a().isInstant())
                                        {
                                            potioneffect.func_188419_a().affectEntity(this, this.func_184494_w(), entitylivingbase, potioneffect.getAmplifier(), 0.5D);
                                        }
                                        else
                                        {
                                            entitylivingbase.addPotionEffect(new PotionEffect(potioneffect));
                                        }
                                    }

                                    if (this.radiusOnUse != 0.0F)
                                    {
                                        f += this.radiusOnUse;

                                        if (f < 0.5F)
                                        {
                                            this.setDead();
                                            return;
                                        }

                                        this.setRadius(f);
                                    }

                                    if (this.durationOnUse != 0)
                                    {
                                        this.duration += this.durationOnUse;

                                        if (this.duration <= 0)
                                        {
                                            this.setDead();
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    public void func_184495_b(float p_184495_1_)
    {
        this.radiusOnUse = p_184495_1_;
    }

    public void func_184487_c(float p_184487_1_)
    {
        this.radiusPerTick = p_184487_1_;
    }

    public void func_184485_d(int p_184485_1_)
    {
        this.waitTime = p_184485_1_;
    }

    public void func_184481_a(EntityLivingBase p_184481_1_)
    {
        this.field_184512_ay = p_184481_1_;
        this.ownerUniqueId = p_184481_1_ == null ? null : p_184481_1_.getUniqueID();
    }

    public EntityLivingBase func_184494_w()
    {
        if (this.field_184512_ay == null && this.ownerUniqueId != null && this.worldObj instanceof WorldServer)
        {
            Entity entity = ((WorldServer)this.worldObj).getEntityFromUuid(this.ownerUniqueId);

            if (entity instanceof EntityLivingBase)
            {
                this.field_184512_ay = (EntityLivingBase)entity;
            }
        }

        return this.field_184512_ay;
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        this.ticksExisted = tagCompund.getInteger("Age");
        this.duration = tagCompund.getInteger("Duration");
        this.waitTime = tagCompund.getInteger("WaitTime");
        this.reapplicationDelay = tagCompund.getInteger("ReapplicationDelay");
        this.durationOnUse = tagCompund.getInteger("DurationOnUse");
        this.radiusOnUse = tagCompund.getFloat("RadiusOnUse");
        this.radiusPerTick = tagCompund.getFloat("RadiusPerTick");
        this.setRadius(tagCompund.getFloat("Radius"));
        this.ownerUniqueId = tagCompund.getUniqueId("OwnerUUID");

        if (tagCompund.hasKey("Particle", 8))
        {
            EnumParticleTypes enumparticletypes = EnumParticleTypes.func_186831_a(tagCompund.getString("Particle"));

            if (enumparticletypes != null)
            {
                this.func_184491_a(enumparticletypes);
            }
        }

        if (tagCompund.hasKey("Color", 99))
        {
            this.func_184482_a(tagCompund.getInteger("Color"));
        }

        if (tagCompund.hasKey("Potion", 8))
        {
            this.func_184484_a(PotionUtils.func_185187_c(tagCompund));
        }

        if (tagCompund.hasKey("Effects", 9))
        {
            NBTTagList nbttaglist = tagCompund.getTagList("Effects", 10);
            this.field_184503_f.clear();

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                PotionEffect potioneffect = PotionEffect.readCustomPotionEffectFromNBT(nbttaglist.getCompoundTagAt(i));

                if (potioneffect != null)
                {
                    this.func_184496_a(potioneffect);
                }
            }
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        tagCompound.setInteger("Age", this.ticksExisted);
        tagCompound.setInteger("Duration", this.duration);
        tagCompound.setInteger("WaitTime", this.waitTime);
        tagCompound.setInteger("ReapplicationDelay", this.reapplicationDelay);
        tagCompound.setInteger("DurationOnUse", this.durationOnUse);
        tagCompound.setFloat("RadiusOnUse", this.radiusOnUse);
        tagCompound.setFloat("RadiusPerTick", this.radiusPerTick);
        tagCompound.setFloat("Radius", this.getRadius());
        tagCompound.setString("Particle", this.func_184493_l().getParticleName());

        if (this.ownerUniqueId != null)
        {
            tagCompound.setUniqueId("OwnerUUID", this.ownerUniqueId);
        }

        if (this.field_184508_au)
        {
            tagCompound.setInteger("Color", this.func_184492_k());
        }

        if (this.field_184502_e != PotionTypes.empty && this.field_184502_e != null)
        {
            tagCompound.setString("Potion", ((ResourceLocation)PotionType.REGISTRY.getNameForObject(this.field_184502_e)).toString());
        }

        if (!this.field_184503_f.isEmpty())
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (PotionEffect potioneffect : this.field_184503_f)
            {
                nbttaglist.appendTag(potioneffect.writeCustomPotionEffectToNBT(new NBTTagCompound()));
            }

            tagCompound.setTag("Effects", nbttaglist);
        }
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        if (RADIUS.equals(key))
        {
            this.setRadius(this.getRadius());
        }

        super.notifyDataManagerChange(key);
    }

    public EnumPushReaction getPushReaction()
    {
        return EnumPushReaction.IGNORE;
    }
}
