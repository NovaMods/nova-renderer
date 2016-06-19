package net.minecraft.entity.passive;

import com.google.common.base.Optional;
import com.google.common.base.Predicate;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.block.SoundType;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.EnumCreatureAttribute;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.IJumpingMount;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIFollowParent;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMate;
import net.minecraft.entity.ai.EntityAIPanic;
import net.minecraft.entity.ai.EntityAIRunAroundLikeCrazy;
import net.minecraft.entity.ai.EntityAISkeletonRiders;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.ai.attributes.AttributeModifier;
import net.minecraft.entity.ai.attributes.IAttribute;
import net.minecraft.entity.ai.attributes.IAttributeInstance;
import net.minecraft.entity.ai.attributes.RangedAttribute;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.AnimalChest;
import net.minecraft.inventory.IInvBasic;
import net.minecraft.inventory.InventoryBasic;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.server.management.PreYggdrasilConverter;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.World;

public class EntityHorse extends EntityAnimal implements IInvBasic, IJumpingMount
{
    private static final Predicate<Entity> horseBreedingSelector = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_ instanceof EntityHorse && ((EntityHorse)p_apply_1_).isBreeding();
        }
    };
    private static final IAttribute horseJumpStrength = (new RangedAttribute((IAttribute)null, "horse.jumpStrength", 0.7D, 0.0D, 2.0D)).setDescription("Jump Strength").setShouldWatch(true);
    private static final UUID field_184786_bD = UUID.fromString("556E1665-8B10-40C8-8F9D-CF9B1667F295");
    private static final DataParameter<Byte> STATUS = EntityDataManager.<Byte>createKey(EntityHorse.class, DataSerializers.BYTE);
    private static final DataParameter<Integer> HORSE_TYPE = EntityDataManager.<Integer>createKey(EntityHorse.class, DataSerializers.VARINT);
    private static final DataParameter<Integer> HORSE_VARIANT = EntityDataManager.<Integer>createKey(EntityHorse.class, DataSerializers.VARINT);
    private static final DataParameter<Optional<UUID>> OWNER_UNIQUE_ID = EntityDataManager.<Optional<UUID>>createKey(EntityHorse.class, DataSerializers.OPTIONAL_UNIQUE_ID);
    private static final DataParameter<Integer> HORSE_ARMOR = EntityDataManager.<Integer>createKey(EntityHorse.class, DataSerializers.VARINT);
    private static final String[] horseTextures = new String[] {"textures/entity/horse/horse_white.png", "textures/entity/horse/horse_creamy.png", "textures/entity/horse/horse_chestnut.png", "textures/entity/horse/horse_brown.png", "textures/entity/horse/horse_black.png", "textures/entity/horse/horse_gray.png", "textures/entity/horse/horse_darkbrown.png"};
    private static final String[] HORSE_TEXTURES_ABBR = new String[] {"hwh", "hcr", "hch", "hbr", "hbl", "hgr", "hdb"};
    private static final String[] horseMarkingTextures = new String[] {null, "textures/entity/horse/horse_markings_white.png", "textures/entity/horse/horse_markings_whitefield.png", "textures/entity/horse/horse_markings_whitedots.png", "textures/entity/horse/horse_markings_blackdots.png"};
    private static final String[] HORSE_MARKING_TEXTURES_ABBR = new String[] {"", "wo_", "wmo", "wdo", "bdo"};
    private final EntityAISkeletonRiders field_184792_bN = new EntityAISkeletonRiders(this);
    private int eatingHaystackCounter;
    private int openMouthCounter;
    private int jumpRearingCounter;
    public int field_110278_bp;
    public int field_110279_bq;
    protected boolean horseJumping;
    private AnimalChest horseChest;
    private boolean hasReproduced;

    /**
     * "The higher this value, the more likely the horse is to be tamed next time a player rides it."
     */
    protected int temper;
    protected float jumpPower;
    private boolean field_110294_bI;
    private boolean field_184793_bU;
    private int field_184794_bV = 0;
    private float headLean;
    private float prevHeadLean;
    private float rearingAmount;
    private float prevRearingAmount;
    private float mouthOpenness;
    private float prevMouthOpenness;

    /** Used to determine the sound that the horse should make when it steps */
    private int gallopTime;
    private String texturePrefix;
    private String[] horseTexturesArray = new String[3];
    private boolean field_175508_bO = false;

    public EntityHorse(World worldIn)
    {
        super(worldIn);
        this.setSize(1.3964844F, 1.6F);
        this.isImmuneToFire = false;
        this.setChested(false);
        this.initHorseChest();
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(0, new EntityAISwimming(this));
        this.tasks.addTask(1, new EntityAIPanic(this, 1.2D));
        this.tasks.addTask(1, new EntityAIRunAroundLikeCrazy(this, 1.2D));
        this.tasks.addTask(2, new EntityAIMate(this, 1.0D));
        this.tasks.addTask(4, new EntityAIFollowParent(this, 1.0D));
        this.tasks.addTask(6, new EntityAIWander(this, 0.7D));
        this.tasks.addTask(7, new EntityAIWatchClosest(this, EntityPlayer.class, 6.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(STATUS, Byte.valueOf((byte)0));
        this.dataWatcher.register(HORSE_TYPE, Integer.valueOf(HorseArmorType.HORSE.func_188595_k()));
        this.dataWatcher.register(HORSE_VARIANT, Integer.valueOf(0));
        this.dataWatcher.register(OWNER_UNIQUE_ID, Optional.<UUID>absent());
        this.dataWatcher.register(HORSE_ARMOR, Integer.valueOf(HorseType.NONE.func_188579_a()));
    }

    public void setType(HorseArmorType p_184778_1_)
    {
        this.dataWatcher.set(HORSE_TYPE, Integer.valueOf(p_184778_1_.func_188595_k()));
        this.resetTexturePrefix();
    }

    public HorseArmorType getType()
    {
        return HorseArmorType.func_188591_a(((Integer)this.dataWatcher.get(HORSE_TYPE)).intValue());
    }

    public void setHorseVariant(int variant)
    {
        this.dataWatcher.set(HORSE_VARIANT, Integer.valueOf(variant));
        this.resetTexturePrefix();
    }

    public int getHorseVariant()
    {
        return ((Integer)this.dataWatcher.get(HORSE_VARIANT)).intValue();
    }

    /**
     * Get the name of this object. For players this returns their username
     */
    public String getName()
    {
        return this.hasCustomName() ? this.getCustomNameTag() : this.getType().func_188596_d().getUnformattedText();
    }

    private boolean getHorseWatchableBoolean(int p_110233_1_)
    {
        return (((Byte)this.dataWatcher.get(STATUS)).byteValue() & p_110233_1_) != 0;
    }

    private void setHorseWatchableBoolean(int p_110208_1_, boolean p_110208_2_)
    {
        byte b0 = ((Byte)this.dataWatcher.get(STATUS)).byteValue();

        if (p_110208_2_)
        {
            this.dataWatcher.set(STATUS, Byte.valueOf((byte)(b0 | p_110208_1_)));
        }
        else
        {
            this.dataWatcher.set(STATUS, Byte.valueOf((byte)(b0 & ~p_110208_1_)));
        }
    }

    public boolean isAdultHorse()
    {
        return !this.isChild();
    }

    public boolean isTame()
    {
        return this.getHorseWatchableBoolean(2);
    }

    public boolean func_110253_bW()
    {
        return this.isAdultHorse();
    }

    public UUID getOwnerUniqueId()
    {
        return (UUID)((Optional)this.dataWatcher.get(OWNER_UNIQUE_ID)).orNull();
    }

    public void setOwnerUniqueId(UUID uniqueId)
    {
        this.dataWatcher.set(OWNER_UNIQUE_ID, Optional.fromNullable(uniqueId));
    }

    public float getHorseSize()
    {
        return 0.5F;
    }

    /**
     * "Sets the scale for an ageable entity according to the boolean parameter, which says if it's a child."
     */
    public void setScaleForAge(boolean p_98054_1_)
    {
        if (p_98054_1_)
        {
            this.setScale(this.getHorseSize());
        }
        else
        {
            this.setScale(1.0F);
        }
    }

    public boolean isHorseJumping()
    {
        return this.horseJumping;
    }

    public void setHorseTamed(boolean tamed)
    {
        this.setHorseWatchableBoolean(2, tamed);
    }

    public void setHorseJumping(boolean jumping)
    {
        this.horseJumping = jumping;
    }

    public boolean func_184652_a(EntityPlayer p_184652_1_)
    {
        return !this.getType().func_188602_h() && super.func_184652_a(p_184652_1_);
    }

    protected void func_142017_o(float p_142017_1_)
    {
        if (p_142017_1_ > 6.0F && this.isEatingHaystack())
        {
            this.setEatingHaystack(false);
        }
    }

    public boolean isChested()
    {
        return this.getType().func_188600_f() && this.getHorseWatchableBoolean(8);
    }

    public HorseType func_184783_dl()
    {
        return HorseType.func_188575_a(((Integer)this.dataWatcher.get(HORSE_ARMOR)).intValue());
    }

    public boolean isEatingHaystack()
    {
        return this.getHorseWatchableBoolean(32);
    }

    public boolean isRearing()
    {
        return this.getHorseWatchableBoolean(64);
    }

    public boolean isBreeding()
    {
        return this.getHorseWatchableBoolean(16);
    }

    public boolean getHasReproduced()
    {
        return this.hasReproduced;
    }

    /**
     * Set horse armor stack (for example: new ItemStack(Items.iron_horse_armor))
     */
    public void setHorseArmorStack(ItemStack itemStackIn)
    {
        HorseType horsetype = HorseType.func_188580_a(itemStackIn);
        this.dataWatcher.set(HORSE_ARMOR, Integer.valueOf(horsetype.func_188579_a()));
        this.resetTexturePrefix();

        if (!this.worldObj.isRemote)
        {
            this.getEntityAttribute(SharedMonsterAttributes.ARMOR).func_188479_b(field_184786_bD);
            int i = horsetype.func_188578_c();

            if (i != 0)
            {
                this.getEntityAttribute(SharedMonsterAttributes.ARMOR).applyModifier((new AttributeModifier(field_184786_bD, "Horse armor bonus", (double)i, 0)).setSaved(false));
            }
        }
    }

    public void setBreeding(boolean breeding)
    {
        this.setHorseWatchableBoolean(16, breeding);
    }

    public void setChested(boolean chested)
    {
        this.setHorseWatchableBoolean(8, chested);
    }

    public void setHasReproduced(boolean hasReproducedIn)
    {
        this.hasReproduced = hasReproducedIn;
    }

    public void setHorseSaddled(boolean saddled)
    {
        this.setHorseWatchableBoolean(4, saddled);
    }

    public int getTemper()
    {
        return this.temper;
    }

    public void setTemper(int temperIn)
    {
        this.temper = temperIn;
    }

    public int increaseTemper(int p_110198_1_)
    {
        int i = MathHelper.clamp_int(this.getTemper() + p_110198_1_, 0, this.getMaxTemper());
        this.setTemper(i);
        return i;
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        Entity entity = source.getEntity();
        return this.isBeingRidden() && entity != null && this.isRidingOrBeingRiddenBy(entity) ? false : super.attackEntityFrom(source, amount);
    }

    /**
     * Returns true if this entity should push and be pushed by other entities when colliding.
     */
    public boolean canBePushed()
    {
        return !this.isBeingRidden();
    }

    public boolean prepareChunkForSpawn()
    {
        int i = MathHelper.floor_double(this.posX);
        int j = MathHelper.floor_double(this.posZ);
        this.worldObj.getBiomeGenForCoords(new BlockPos(i, 0, j));
        return true;
    }

    public void dropChests()
    {
        if (!this.worldObj.isRemote && this.isChested())
        {
            this.dropItem(Item.getItemFromBlock(Blocks.chest), 1);
            this.setChested(false);
        }
    }

    private void func_110266_cB()
    {
        this.openHorseMouth();

        if (!this.isSilent())
        {
            this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_horse_eat, this.getSoundCategory(), 1.0F, 1.0F + (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F);
        }
    }

    public void fall(float distance, float damageMultiplier)
    {
        if (distance > 1.0F)
        {
            this.playSound(SoundEvents.entity_horse_land, 0.4F, 1.0F);
        }

        int i = MathHelper.ceiling_float_int((distance * 0.5F - 3.0F) * damageMultiplier);

        if (i > 0)
        {
            this.attackEntityFrom(DamageSource.fall, (float)i);

            if (this.isBeingRidden())
            {
                for (Entity entity : this.func_184182_bu())
                {
                    entity.attackEntityFrom(DamageSource.fall, (float)i);
                }
            }

            IBlockState iblockstate = this.worldObj.getBlockState(new BlockPos(this.posX, this.posY - 0.2D - (double)this.prevRotationYaw, this.posZ));
            Block block = iblockstate.getBlock();

            if (iblockstate.getMaterial() != Material.air && !this.isSilent())
            {
                SoundType soundtype = block.getStepSound();
                this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, soundtype.func_185844_d(), this.getSoundCategory(), soundtype.func_185843_a() * 0.5F, soundtype.func_185847_b() * 0.75F);
            }
        }
    }

    /**
     * Returns number of slots depending horse type
     */
    private int getChestSize()
    {
        HorseArmorType horsearmortype = this.getType();
        return this.isChested() && horsearmortype.func_188600_f() ? 17 : 2;
    }

    private void initHorseChest()
    {
        AnimalChest animalchest = this.horseChest;
        this.horseChest = new AnimalChest("HorseChest", this.getChestSize());
        this.horseChest.setCustomName(this.getName());

        if (animalchest != null)
        {
            animalchest.removeInventoryChangeListener(this);
            int i = Math.min(animalchest.getSizeInventory(), this.horseChest.getSizeInventory());

            for (int j = 0; j < i; ++j)
            {
                ItemStack itemstack = animalchest.getStackInSlot(j);

                if (itemstack != null)
                {
                    this.horseChest.setInventorySlotContents(j, itemstack.copy());
                }
            }
        }

        this.horseChest.addInventoryChangeListener(this);
        this.updateHorseSlots();
    }

    /**
     * Updates the items in the saddle and armor slots of the horse's inventory.
     */
    private void updateHorseSlots()
    {
        if (!this.worldObj.isRemote)
        {
            this.setHorseSaddled(this.horseChest.getStackInSlot(0) != null);

            if (this.getType().func_188603_j())
            {
                this.setHorseArmorStack(this.horseChest.getStackInSlot(1));
            }
        }
    }

    /**
     * Called by InventoryBasic.onInventoryChanged() on a array that is never filled.
     */
    public void onInventoryChanged(InventoryBasic p_76316_1_)
    {
        HorseType horsetype = this.func_184783_dl();
        boolean flag = this.isHorseSaddled();
        this.updateHorseSlots();

        if (this.ticksExisted > 20)
        {
            if (horsetype == HorseType.NONE && horsetype != this.func_184783_dl())
            {
                this.playSound(SoundEvents.entity_horse_armor, 0.5F, 1.0F);
            }
            else if (horsetype != this.func_184783_dl())
            {
                this.playSound(SoundEvents.entity_horse_armor, 0.5F, 1.0F);
            }

            if (!flag && this.isHorseSaddled())
            {
                this.playSound(SoundEvents.entity_horse_saddle, 0.5F, 1.0F);
            }
        }
    }

    /**
     * Checks if the entity's current position is a valid location to spawn this entity.
     */
    public boolean getCanSpawnHere()
    {
        this.prepareChunkForSpawn();
        return super.getCanSpawnHere();
    }

    protected EntityHorse getClosestHorse(Entity entityIn, double distance)
    {
        double d0 = Double.MAX_VALUE;
        Entity entity = null;

        for (Entity entity1 : this.worldObj.getEntitiesInAABBexcluding(entityIn, entityIn.getEntityBoundingBox().addCoord(distance, distance, distance), horseBreedingSelector))
        {
            double d1 = entity1.getDistanceSq(entityIn.posX, entityIn.posY, entityIn.posZ);

            if (d1 < d0)
            {
                entity = entity1;
                d0 = d1;
            }
        }

        return (EntityHorse)entity;
    }

    public double getHorseJumpStrength()
    {
        return this.getEntityAttribute(horseJumpStrength).getAttributeValue();
    }

    protected SoundEvent getDeathSound()
    {
        this.openHorseMouth();
        return this.getType().func_188593_c();
    }

    protected SoundEvent getHurtSound()
    {
        this.openHorseMouth();

        if (this.rand.nextInt(3) == 0)
        {
            this.makeHorseRear();
        }

        return this.getType().func_188597_b();
    }

    public boolean isHorseSaddled()
    {
        return this.getHorseWatchableBoolean(4);
    }

    protected SoundEvent getAmbientSound()
    {
        this.openHorseMouth();

        if (this.rand.nextInt(10) == 0 && !this.isMovementBlocked())
        {
            this.makeHorseRear();
        }

        return this.getType().func_188599_a();
    }

    protected SoundEvent func_184785_dv()
    {
        this.openHorseMouth();
        this.makeHorseRear();
        HorseArmorType horsearmortype = this.getType();
        return horsearmortype.func_188602_h() ? null : (horsearmortype.func_188601_g() ? SoundEvents.entity_donkey_angry : SoundEvents.entity_horse_angry);
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        SoundType soundtype = blockIn.getStepSound();

        if (this.worldObj.getBlockState(pos.up()).getBlock() == Blocks.snow_layer)
        {
            soundtype = Blocks.snow_layer.getStepSound();
        }

        if (!blockIn.getDefaultState().getMaterial().isLiquid())
        {
            HorseArmorType horsearmortype = this.getType();

            if (this.isBeingRidden() && !horsearmortype.func_188601_g())
            {
                ++this.gallopTime;

                if (this.gallopTime > 5 && this.gallopTime % 3 == 0)
                {
                    this.playSound(SoundEvents.entity_horse_gallop, soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());

                    if (horsearmortype == HorseArmorType.HORSE && this.rand.nextInt(10) == 0)
                    {
                        this.playSound(SoundEvents.entity_horse_breathe, soundtype.func_185843_a() * 0.6F, soundtype.func_185847_b());
                    }
                }
                else if (this.gallopTime <= 5)
                {
                    this.playSound(SoundEvents.entity_horse_step_wood, soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());
                }
            }
            else if (soundtype == SoundType.WOOD)
            {
                this.playSound(SoundEvents.entity_horse_step_wood, soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());
            }
            else
            {
                this.playSound(SoundEvents.entity_horse_step, soundtype.func_185843_a() * 0.15F, soundtype.func_185847_b());
            }
        }
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getAttributeMap().registerAttribute(horseJumpStrength);
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(53.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.22499999403953552D);
    }

    /**
     * Will return how many at most can spawn in a chunk at once.
     */
    public int getMaxSpawnedInChunk()
    {
        return 6;
    }

    public int getMaxTemper()
    {
        return 100;
    }

    /**
     * Returns the volume for the sounds this mob makes.
     */
    protected float getSoundVolume()
    {
        return 0.8F;
    }

    /**
     * Get number of ticks, at least during which the living entity will be silent.
     */
    public int getTalkInterval()
    {
        return 400;
    }

    public boolean func_110239_cn()
    {
        return this.getType() == HorseArmorType.HORSE || this.func_184783_dl() != HorseType.NONE;
    }

    private void resetTexturePrefix()
    {
        this.texturePrefix = null;
    }

    public boolean func_175507_cI()
    {
        return this.field_175508_bO;
    }

    private void setHorseTexturePaths()
    {
        this.texturePrefix = "horse/";
        this.horseTexturesArray[0] = null;
        this.horseTexturesArray[1] = null;
        this.horseTexturesArray[2] = null;
        HorseArmorType horsearmortype = this.getType();
        int i = this.getHorseVariant();

        if (horsearmortype == HorseArmorType.HORSE)
        {
            int j = i & 255;
            int k = (i & 65280) >> 8;

            if (j >= horseTextures.length)
            {
                this.field_175508_bO = false;
                return;
            }

            this.horseTexturesArray[0] = horseTextures[j];
            this.texturePrefix = this.texturePrefix + HORSE_TEXTURES_ABBR[j];

            if (k >= horseMarkingTextures.length)
            {
                this.field_175508_bO = false;
                return;
            }

            this.horseTexturesArray[1] = horseMarkingTextures[k];
            this.texturePrefix = this.texturePrefix + HORSE_MARKING_TEXTURES_ABBR[k];
        }
        else
        {
            this.horseTexturesArray[0] = "";
            this.texturePrefix = this.texturePrefix + "_" + horsearmortype + "_";
        }

        HorseType horsetype = this.func_184783_dl();
        this.horseTexturesArray[2] = horsetype.func_188574_d();
        this.texturePrefix = this.texturePrefix + horsetype.func_188573_b();
        this.field_175508_bO = true;
    }

    public String getHorseTexture()
    {
        if (this.texturePrefix == null)
        {
            this.setHorseTexturePaths();
        }

        return this.texturePrefix;
    }

    public String[] getVariantTexturePaths()
    {
        if (this.texturePrefix == null)
        {
            this.setHorseTexturePaths();
        }

        return this.horseTexturesArray;
    }

    public void openGUI(EntityPlayer playerEntity)
    {
        if (!this.worldObj.isRemote && (!this.isBeingRidden() || this.isPassenger(playerEntity)) && this.isTame())
        {
            this.horseChest.setCustomName(this.getName());
            playerEntity.func_184826_a(this, this.horseChest);
        }
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.spawn_egg)
        {
            return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
        }
        else if (!this.isTame() && this.getType().func_188602_h())
        {
            return false;
        }
        else if (this.isTame() && this.isAdultHorse() && p_184645_1_.isSneaking())
        {
            this.openGUI(p_184645_1_);
            return true;
        }
        else if (this.func_110253_bW() && this.isBeingRidden())
        {
            return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
        }
        else
        {
            if (p_184645_3_ != null)
            {
                if (this.getType().func_188603_j())
                {
                    HorseType horsetype = HorseType.func_188580_a(p_184645_3_);

                    if (horsetype != HorseType.NONE)
                    {
                        if (!this.isTame())
                        {
                            this.makeHorseRearWithSound();
                            return true;
                        }

                        this.openGUI(p_184645_1_);
                        return true;
                    }
                }

                boolean flag = false;

                if (!this.getType().func_188602_h())
                {
                    float f = 0.0F;
                    int i = 0;
                    int j = 0;

                    if (p_184645_3_.getItem() == Items.wheat)
                    {
                        f = 2.0F;
                        i = 20;
                        j = 3;
                    }
                    else if (p_184645_3_.getItem() == Items.sugar)
                    {
                        f = 1.0F;
                        i = 30;
                        j = 3;
                    }
                    else if (Block.getBlockFromItem(p_184645_3_.getItem()) == Blocks.hay_block)
                    {
                        f = 20.0F;
                        i = 180;
                    }
                    else if (p_184645_3_.getItem() == Items.apple)
                    {
                        f = 3.0F;
                        i = 60;
                        j = 3;
                    }
                    else if (p_184645_3_.getItem() == Items.golden_carrot)
                    {
                        f = 4.0F;
                        i = 60;
                        j = 5;

                        if (this.isTame() && this.getGrowingAge() == 0)
                        {
                            flag = true;
                            this.setInLove(p_184645_1_);
                        }
                    }
                    else if (p_184645_3_.getItem() == Items.golden_apple)
                    {
                        f = 10.0F;
                        i = 240;
                        j = 10;

                        if (this.isTame() && this.getGrowingAge() == 0 && !this.isInLove())
                        {
                            flag = true;
                            this.setInLove(p_184645_1_);
                        }
                    }

                    if (this.getHealth() < this.getMaxHealth() && f > 0.0F)
                    {
                        this.heal(f);
                        flag = true;
                    }

                    if (!this.isAdultHorse() && i > 0)
                    {
                        if (!this.worldObj.isRemote)
                        {
                            this.addGrowth(i);
                        }

                        flag = true;
                    }

                    if (j > 0 && (flag || !this.isTame()) && this.getTemper() < this.getMaxTemper())
                    {
                        flag = true;

                        if (!this.worldObj.isRemote)
                        {
                            this.increaseTemper(j);
                        }
                    }

                    if (flag)
                    {
                        this.func_110266_cB();
                    }
                }

                if (!this.isTame() && !flag)
                {
                    if (p_184645_3_.interactWithEntity(p_184645_1_, this, p_184645_2_))
                    {
                        return true;
                    }

                    this.makeHorseRearWithSound();
                    return true;
                }

                if (!flag && this.getType().func_188600_f() && !this.isChested() && p_184645_3_.getItem() == Item.getItemFromBlock(Blocks.chest))
                {
                    this.setChested(true);
                    this.playSound(SoundEvents.entity_donkey_chest, 1.0F, (this.rand.nextFloat() - this.rand.nextFloat()) * 0.2F + 1.0F);
                    flag = true;
                    this.initHorseChest();
                }

                if (!flag && this.func_110253_bW() && !this.isHorseSaddled() && p_184645_3_.getItem() == Items.saddle)
                {
                    this.openGUI(p_184645_1_);
                    return true;
                }

                if (flag)
                {
                    if (!p_184645_1_.capabilities.isCreativeMode)
                    {
                        --p_184645_3_.stackSize;
                    }

                    return true;
                }
            }

            if (this.func_110253_bW() && !this.isBeingRidden())
            {
                if (p_184645_3_ != null && p_184645_3_.interactWithEntity(p_184645_1_, this, p_184645_2_))
                {
                    return true;
                }
                else
                {
                    this.mountTo(p_184645_1_);
                    return true;
                }
            }
            else
            {
                return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
            }
        }
    }

    private void mountTo(EntityPlayer player)
    {
        player.rotationYaw = this.rotationYaw;
        player.rotationPitch = this.rotationPitch;
        this.setEatingHaystack(false);
        this.setRearing(false);

        if (!this.worldObj.isRemote)
        {
            player.startRiding(this);
        }
    }

    /**
     * Dead and sleeping entities cannot move
     */
    protected boolean isMovementBlocked()
    {
        return this.isBeingRidden() && this.isHorseSaddled() ? true : this.isEatingHaystack() || this.isRearing();
    }

    /**
     * Checks if the parameter is an item which this animal can be fed to breed it (wheat, carrots or seeds depending on
     * the animal type)
     */
    public boolean isBreedingItem(ItemStack stack)
    {
        return false;
    }

    private void func_110210_cH()
    {
        this.field_110278_bp = 1;
    }

    /**
     * Called when the mob's health reaches 0.
     */
    public void onDeath(DamageSource cause)
    {
        super.onDeath(cause);

        if (!this.worldObj.isRemote)
        {
            this.dropChestItems();
        }
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.rand.nextInt(200) == 0)
        {
            this.func_110210_cH();
        }

        super.onLivingUpdate();

        if (!this.worldObj.isRemote)
        {
            if (this.rand.nextInt(900) == 0 && this.deathTime == 0)
            {
                this.heal(1.0F);
            }

            if (!this.isEatingHaystack() && !this.isBeingRidden() && this.rand.nextInt(300) == 0 && this.worldObj.getBlockState(new BlockPos(MathHelper.floor_double(this.posX), MathHelper.floor_double(this.posY) - 1, MathHelper.floor_double(this.posZ))).getBlock() == Blocks.grass)
            {
                this.setEatingHaystack(true);
            }

            if (this.isEatingHaystack() && ++this.eatingHaystackCounter > 50)
            {
                this.eatingHaystackCounter = 0;
                this.setEatingHaystack(false);
            }

            if (this.isBreeding() && !this.isAdultHorse() && !this.isEatingHaystack())
            {
                EntityHorse entityhorse = this.getClosestHorse(this, 16.0D);

                if (entityhorse != null && this.getDistanceSqToEntity(entityhorse) > 4.0D)
                {
                    this.navigator.getPathToEntityLiving(entityhorse);
                }
            }

            if (this.func_184782_dG() && this.field_184794_bV++ >= 18000)
            {
                this.setDead();
            }
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();

        if (this.worldObj.isRemote && this.dataWatcher.isDirty())
        {
            this.dataWatcher.setClean();
            this.resetTexturePrefix();
        }

        if (this.openMouthCounter > 0 && ++this.openMouthCounter > 30)
        {
            this.openMouthCounter = 0;
            this.setHorseWatchableBoolean(128, false);
        }

        if (this.func_184186_bw() && this.jumpRearingCounter > 0 && ++this.jumpRearingCounter > 20)
        {
            this.jumpRearingCounter = 0;
            this.setRearing(false);
        }

        if (this.field_110278_bp > 0 && ++this.field_110278_bp > 8)
        {
            this.field_110278_bp = 0;
        }

        if (this.field_110279_bq > 0)
        {
            ++this.field_110279_bq;

            if (this.field_110279_bq > 300)
            {
                this.field_110279_bq = 0;
            }
        }

        this.prevHeadLean = this.headLean;

        if (this.isEatingHaystack())
        {
            this.headLean += (1.0F - this.headLean) * 0.4F + 0.05F;

            if (this.headLean > 1.0F)
            {
                this.headLean = 1.0F;
            }
        }
        else
        {
            this.headLean += (0.0F - this.headLean) * 0.4F - 0.05F;

            if (this.headLean < 0.0F)
            {
                this.headLean = 0.0F;
            }
        }

        this.prevRearingAmount = this.rearingAmount;

        if (this.isRearing())
        {
            this.prevHeadLean = this.headLean = 0.0F;
            this.rearingAmount += (1.0F - this.rearingAmount) * 0.4F + 0.05F;

            if (this.rearingAmount > 1.0F)
            {
                this.rearingAmount = 1.0F;
            }
        }
        else
        {
            this.field_110294_bI = false;
            this.rearingAmount += (0.8F * this.rearingAmount * this.rearingAmount * this.rearingAmount - this.rearingAmount) * 0.6F - 0.05F;

            if (this.rearingAmount < 0.0F)
            {
                this.rearingAmount = 0.0F;
            }
        }

        this.prevMouthOpenness = this.mouthOpenness;

        if (this.getHorseWatchableBoolean(128))
        {
            this.mouthOpenness += (1.0F - this.mouthOpenness) * 0.7F + 0.05F;

            if (this.mouthOpenness > 1.0F)
            {
                this.mouthOpenness = 1.0F;
            }
        }
        else
        {
            this.mouthOpenness += (0.0F - this.mouthOpenness) * 0.7F - 0.05F;

            if (this.mouthOpenness < 0.0F)
            {
                this.mouthOpenness = 0.0F;
            }
        }
    }

    private void openHorseMouth()
    {
        if (!this.worldObj.isRemote)
        {
            this.openMouthCounter = 1;
            this.setHorseWatchableBoolean(128, true);
        }
    }

    /**
     * Return true if the horse entity ready to mate. (no rider, not riding, tame, adult, not steril...)
     */
    private boolean canMate()
    {
        return !this.isBeingRidden() && !this.isRiding() && this.isTame() && this.isAdultHorse() && this.getType().func_188590_i() && this.getHealth() >= this.getMaxHealth() && this.isInLove();
    }

    public void setEatingHaystack(boolean p_110227_1_)
    {
        this.setHorseWatchableBoolean(32, p_110227_1_);
    }

    public void setRearing(boolean rearing)
    {
        if (rearing)
        {
            this.setEatingHaystack(false);
        }

        this.setHorseWatchableBoolean(64, rearing);
    }

    private void makeHorseRear()
    {
        if (this.func_184186_bw())
        {
            this.jumpRearingCounter = 1;
            this.setRearing(true);
        }
    }

    public void makeHorseRearWithSound()
    {
        this.makeHorseRear();
        SoundEvent soundevent = this.func_184785_dv();

        if (soundevent != null)
        {
            this.playSound(soundevent, this.getSoundVolume(), this.getSoundPitch());
        }
    }

    public void dropChestItems()
    {
        this.dropItemsInChest(this, this.horseChest);
        this.dropChests();
    }

    private void dropItemsInChest(Entity entityIn, AnimalChest animalChestIn)
    {
        if (animalChestIn != null && !this.worldObj.isRemote)
        {
            for (int i = 0; i < animalChestIn.getSizeInventory(); ++i)
            {
                ItemStack itemstack = animalChestIn.getStackInSlot(i);

                if (itemstack != null)
                {
                    this.entityDropItem(itemstack, 0.0F);
                }
            }
        }
    }

    public boolean setTamedBy(EntityPlayer player)
    {
        this.setOwnerUniqueId(player.getUniqueID());
        this.setHorseTamed(true);
        return true;
    }

    /**
     * Moves the entity based on the specified heading.  Args: strafe, forward
     */
    public void moveEntityWithHeading(float strafe, float forward)
    {
        if (this.isBeingRidden() && this.canBeSteered() && this.isHorseSaddled())
        {
            EntityLivingBase entitylivingbase = (EntityLivingBase)this.getControllingPassenger();
            this.prevRotationYaw = this.rotationYaw = entitylivingbase.rotationYaw;
            this.rotationPitch = entitylivingbase.rotationPitch * 0.5F;
            this.setRotation(this.rotationYaw, this.rotationPitch);
            this.rotationYawHead = this.renderYawOffset = this.rotationYaw;
            strafe = entitylivingbase.moveStrafing * 0.5F;
            forward = entitylivingbase.moveForward;

            if (forward <= 0.0F)
            {
                forward *= 0.25F;
                this.gallopTime = 0;
            }

            if (this.onGround && this.jumpPower == 0.0F && this.isRearing() && !this.field_110294_bI)
            {
                strafe = 0.0F;
                forward = 0.0F;
            }

            if (this.jumpPower > 0.0F && !this.isHorseJumping() && this.onGround)
            {
                this.motionY = this.getHorseJumpStrength() * (double)this.jumpPower;

                if (this.isPotionActive(MobEffects.jump))
                {
                    this.motionY += (double)((float)(this.getActivePotionEffect(MobEffects.jump).getAmplifier() + 1) * 0.1F);
                }

                this.setHorseJumping(true);
                this.isAirBorne = true;

                if (forward > 0.0F)
                {
                    float f = MathHelper.sin(this.rotationYaw * 0.017453292F);
                    float f1 = MathHelper.cos(this.rotationYaw * 0.017453292F);
                    this.motionX += (double)(-0.4F * f * this.jumpPower);
                    this.motionZ += (double)(0.4F * f1 * this.jumpPower);
                    this.playSound(SoundEvents.entity_horse_jump, 0.4F, 1.0F);
                }

                this.jumpPower = 0.0F;
            }

            this.stepHeight = 1.0F;
            this.jumpMovementFactor = this.getAIMoveSpeed() * 0.1F;

            if (this.func_184186_bw())
            {
                this.setAIMoveSpeed((float)this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getAttributeValue());
                super.moveEntityWithHeading(strafe, forward);
            }
            else if (entitylivingbase instanceof EntityPlayer)
            {
                this.motionX = 0.0D;
                this.motionY = 0.0D;
                this.motionZ = 0.0D;
            }

            if (this.onGround)
            {
                this.jumpPower = 0.0F;
                this.setHorseJumping(false);
            }

            this.field_184618_aE = this.limbSwingAmount;
            double d1 = this.posX - this.prevPosX;
            double d0 = this.posZ - this.prevPosZ;
            float f2 = MathHelper.sqrt_double(d1 * d1 + d0 * d0) * 4.0F;

            if (f2 > 1.0F)
            {
                f2 = 1.0F;
            }

            this.limbSwingAmount += (f2 - this.limbSwingAmount) * 0.4F;
            this.field_184619_aG += this.limbSwingAmount;
        }
        else
        {
            this.stepHeight = 0.5F;
            this.jumpMovementFactor = 0.02F;
            super.moveEntityWithHeading(strafe, forward);
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("EatingHaystack", this.isEatingHaystack());
        tagCompound.setBoolean("ChestedHorse", this.isChested());
        tagCompound.setBoolean("HasReproduced", this.getHasReproduced());
        tagCompound.setBoolean("Bred", this.isBreeding());
        tagCompound.setInteger("Type", this.getType().func_188595_k());
        tagCompound.setInteger("Variant", this.getHorseVariant());
        tagCompound.setInteger("Temper", this.getTemper());
        tagCompound.setBoolean("Tame", this.isTame());
        tagCompound.setBoolean("SkeletonTrap", this.func_184782_dG());
        tagCompound.setInteger("SkeletonTrapTime", this.field_184794_bV);

        if (this.getOwnerUniqueId() != null)
        {
            tagCompound.setString("OwnerUUID", this.getOwnerUniqueId().toString());
        }

        if (this.isChested())
        {
            NBTTagList nbttaglist = new NBTTagList();

            for (int i = 2; i < this.horseChest.getSizeInventory(); ++i)
            {
                ItemStack itemstack = this.horseChest.getStackInSlot(i);

                if (itemstack != null)
                {
                    NBTTagCompound nbttagcompound = new NBTTagCompound();
                    nbttagcompound.setByte("Slot", (byte)i);
                    itemstack.writeToNBT(nbttagcompound);
                    nbttaglist.appendTag(nbttagcompound);
                }
            }

            tagCompound.setTag("Items", nbttaglist);
        }

        if (this.horseChest.getStackInSlot(1) != null)
        {
            tagCompound.setTag("ArmorItem", this.horseChest.getStackInSlot(1).writeToNBT(new NBTTagCompound()));
        }

        if (this.horseChest.getStackInSlot(0) != null)
        {
            tagCompound.setTag("SaddleItem", this.horseChest.getStackInSlot(0).writeToNBT(new NBTTagCompound()));
        }
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setEatingHaystack(tagCompund.getBoolean("EatingHaystack"));
        this.setBreeding(tagCompund.getBoolean("Bred"));
        this.setChested(tagCompund.getBoolean("ChestedHorse"));
        this.setHasReproduced(tagCompund.getBoolean("HasReproduced"));
        this.setType(HorseArmorType.func_188591_a(tagCompund.getInteger("Type")));
        this.setHorseVariant(tagCompund.getInteger("Variant"));
        this.setTemper(tagCompund.getInteger("Temper"));
        this.setHorseTamed(tagCompund.getBoolean("Tame"));
        this.func_184784_x(tagCompund.getBoolean("SkeletonTrap"));
        this.field_184794_bV = tagCompund.getInteger("SkeletonTrapTime");
        String s = "";

        if (tagCompund.hasKey("OwnerUUID", 8))
        {
            s = tagCompund.getString("OwnerUUID");
        }
        else
        {
            String s1 = tagCompund.getString("Owner");
            s = PreYggdrasilConverter.func_187473_a(this.getServer(), s1);
        }

        if (!s.isEmpty())
        {
            this.setOwnerUniqueId(UUID.fromString(s));
        }

        IAttributeInstance iattributeinstance = this.getAttributeMap().getAttributeInstanceByName("Speed");

        if (iattributeinstance != null)
        {
            this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(iattributeinstance.getBaseValue() * 0.25D);
        }

        if (this.isChested())
        {
            NBTTagList nbttaglist = tagCompund.getTagList("Items", 10);
            this.initHorseChest();

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                NBTTagCompound nbttagcompound = nbttaglist.getCompoundTagAt(i);
                int j = nbttagcompound.getByte("Slot") & 255;

                if (j >= 2 && j < this.horseChest.getSizeInventory())
                {
                    this.horseChest.setInventorySlotContents(j, ItemStack.loadItemStackFromNBT(nbttagcompound));
                }
            }
        }

        if (tagCompund.hasKey("ArmorItem", 10))
        {
            ItemStack itemstack = ItemStack.loadItemStackFromNBT(tagCompund.getCompoundTag("ArmorItem"));

            if (itemstack != null && HorseType.func_188577_b(itemstack.getItem()))
            {
                this.horseChest.setInventorySlotContents(1, itemstack);
            }
        }

        if (tagCompund.hasKey("SaddleItem", 10))
        {
            ItemStack itemstack1 = ItemStack.loadItemStackFromNBT(tagCompund.getCompoundTag("SaddleItem"));

            if (itemstack1 != null && itemstack1.getItem() == Items.saddle)
            {
                this.horseChest.setInventorySlotContents(0, itemstack1);
            }
        }

        this.updateHorseSlots();
    }

    /**
     * Returns true if the mob is currently able to mate with the specified mob.
     */
    public boolean canMateWith(EntityAnimal otherAnimal)
    {
        if (otherAnimal == this)
        {
            return false;
        }
        else if (otherAnimal.getClass() != this.getClass())
        {
            return false;
        }
        else
        {
            EntityHorse entityhorse = (EntityHorse)otherAnimal;

            if (this.canMate() && entityhorse.canMate())
            {
                HorseArmorType horsearmortype = this.getType();
                HorseArmorType horsearmortype1 = entityhorse.getType();
                return horsearmortype == horsearmortype1 || horsearmortype == HorseArmorType.HORSE && horsearmortype1 == HorseArmorType.DONKEY || horsearmortype == HorseArmorType.DONKEY && horsearmortype1 == HorseArmorType.HORSE;
            }
            else
            {
                return false;
            }
        }
    }

    public EntityAgeable createChild(EntityAgeable ageable)
    {
        EntityHorse entityhorse = (EntityHorse)ageable;
        EntityHorse entityhorse1 = new EntityHorse(this.worldObj);
        HorseArmorType horsearmortype = this.getType();
        HorseArmorType horsearmortype1 = entityhorse.getType();
        HorseArmorType horsearmortype2 = HorseArmorType.HORSE;

        if (horsearmortype == horsearmortype1)
        {
            horsearmortype2 = horsearmortype;
        }
        else if (horsearmortype == HorseArmorType.HORSE && horsearmortype1 == HorseArmorType.DONKEY || horsearmortype == HorseArmorType.DONKEY && horsearmortype1 == HorseArmorType.HORSE)
        {
            horsearmortype2 = HorseArmorType.MULE;
        }

        if (horsearmortype2 == HorseArmorType.HORSE)
        {
            int j = this.rand.nextInt(9);
            int i;

            if (j < 4)
            {
                i = this.getHorseVariant() & 255;
            }
            else if (j < 8)
            {
                i = entityhorse.getHorseVariant() & 255;
            }
            else
            {
                i = this.rand.nextInt(7);
            }

            int k = this.rand.nextInt(5);

            if (k < 2)
            {
                i = i | this.getHorseVariant() & 65280;
            }
            else if (k < 4)
            {
                i = i | entityhorse.getHorseVariant() & 65280;
            }
            else
            {
                i = i | this.rand.nextInt(5) << 8 & 65280;
            }

            entityhorse1.setHorseVariant(i);
        }

        entityhorse1.setType(horsearmortype2);
        double d1 = this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).getBaseValue() + ageable.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).getBaseValue() + (double)this.getModifiedMaxHealth();
        entityhorse1.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(d1 / 3.0D);
        double d2 = this.getEntityAttribute(horseJumpStrength).getBaseValue() + ageable.getEntityAttribute(horseJumpStrength).getBaseValue() + this.getModifiedJumpStrength();
        entityhorse1.getEntityAttribute(horseJumpStrength).setBaseValue(d2 / 3.0D);
        double d0 = this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getBaseValue() + ageable.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).getBaseValue() + this.getModifiedMovementSpeed();
        entityhorse1.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(d0 / 3.0D);
        return entityhorse1;
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        livingdata = super.onInitialSpawn(difficulty, livingdata);
        HorseArmorType horsearmortype = HorseArmorType.HORSE;
        int i = 0;

        if (livingdata instanceof EntityHorse.GroupData)
        {
            horsearmortype = ((EntityHorse.GroupData)livingdata).field_188476_a;
            i = ((EntityHorse.GroupData)livingdata).field_188477_b & 255 | this.rand.nextInt(5) << 8;
        }
        else
        {
            if (this.rand.nextInt(10) == 0)
            {
                horsearmortype = HorseArmorType.DONKEY;
            }
            else
            {
                int j = this.rand.nextInt(7);
                int k = this.rand.nextInt(5);
                horsearmortype = HorseArmorType.HORSE;
                i = j | k << 8;
            }

            livingdata = new EntityHorse.GroupData(horsearmortype, i);
        }

        this.setType(horsearmortype);
        this.setHorseVariant(i);

        if (this.rand.nextInt(5) == 0)
        {
            this.setGrowingAge(-24000);
        }

        if (horsearmortype.func_188602_h())
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(15.0D);
            this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.20000000298023224D);
        }
        else
        {
            this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue((double)this.getModifiedMaxHealth());

            if (horsearmortype == HorseArmorType.HORSE)
            {
                this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(this.getModifiedMovementSpeed());
            }
            else
            {
                this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.17499999701976776D);
            }
        }

        if (horsearmortype.func_188601_g())
        {
            this.getEntityAttribute(horseJumpStrength).setBaseValue(0.5D);
        }
        else
        {
            this.getEntityAttribute(horseJumpStrength).setBaseValue(this.getModifiedJumpStrength());
        }

        this.setHealth(this.getMaxHealth());
        return livingdata;
    }

    /**
     * returns true if all the conditions for steering the entity are met. For pigs, this is true if it is being ridden
     * by a player and the player is holding a carrot-on-a-stick
     */
    public boolean canBeSteered()
    {
        Entity entity = this.getControllingPassenger();
        return entity instanceof EntityLivingBase;
    }

    public float getGrassEatingAmount(float p_110258_1_)
    {
        return this.prevHeadLean + (this.headLean - this.prevHeadLean) * p_110258_1_;
    }

    public float getRearingAmount(float p_110223_1_)
    {
        return this.prevRearingAmount + (this.rearingAmount - this.prevRearingAmount) * p_110223_1_;
    }

    public float getMouthOpennessAngle(float p_110201_1_)
    {
        return this.prevMouthOpenness + (this.mouthOpenness - this.prevMouthOpenness) * p_110201_1_;
    }

    public void setJumpPower(int jumpPowerIn)
    {
        if (this.isHorseSaddled())
        {
            if (jumpPowerIn < 0)
            {
                jumpPowerIn = 0;
            }
            else
            {
                this.field_110294_bI = true;
                this.makeHorseRear();
            }

            if (jumpPowerIn >= 90)
            {
                this.jumpPower = 1.0F;
            }
            else
            {
                this.jumpPower = 0.4F + 0.4F * (float)jumpPowerIn / 90.0F;
            }
        }
    }

    public boolean func_184776_b()
    {
        return this.isHorseSaddled();
    }

    public void func_184775_b(int p_184775_1_)
    {
        this.field_110294_bI = true;
        this.makeHorseRear();
    }

    public void func_184777_r_()
    {
    }

    /**
     * "Spawns particles for the horse entity. par1 tells whether to spawn hearts. If it is false, it spawns smoke."
     */
    protected void spawnHorseParticles(boolean p_110216_1_)
    {
        EnumParticleTypes enumparticletypes = p_110216_1_ ? EnumParticleTypes.HEART : EnumParticleTypes.SMOKE_NORMAL;

        for (int i = 0; i < 7; ++i)
        {
            double d0 = this.rand.nextGaussian() * 0.02D;
            double d1 = this.rand.nextGaussian() * 0.02D;
            double d2 = this.rand.nextGaussian() * 0.02D;
            this.worldObj.spawnParticle(enumparticletypes, this.posX + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, this.posY + 0.5D + (double)(this.rand.nextFloat() * this.height), this.posZ + (double)(this.rand.nextFloat() * this.width * 2.0F) - (double)this.width, d0, d1, d2, new int[0]);
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 7)
        {
            this.spawnHorseParticles(true);
        }
        else if (id == 6)
        {
            this.spawnHorseParticles(false);
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public void updatePassenger(Entity passenger)
    {
        super.updatePassenger(passenger);

        if (passenger instanceof EntityLiving)
        {
            EntityLiving entityliving = (EntityLiving)passenger;
            this.renderYawOffset = entityliving.renderYawOffset;
        }

        if (this.prevRearingAmount > 0.0F)
        {
            float f3 = MathHelper.sin(this.renderYawOffset * 0.017453292F);
            float f = MathHelper.cos(this.renderYawOffset * 0.017453292F);
            float f1 = 0.7F * this.prevRearingAmount;
            float f2 = 0.15F * this.prevRearingAmount;
            passenger.setPosition(this.posX + (double)(f1 * f3), this.posY + this.getMountedYOffset() + passenger.getYOffset() + (double)f2, this.posZ - (double)(f1 * f));

            if (passenger instanceof EntityLivingBase)
            {
                ((EntityLivingBase)passenger).renderYawOffset = this.renderYawOffset;
            }
        }
    }

    /**
     * Returns the Y offset from the entity's position for any entity riding this one.
     */
    public double getMountedYOffset()
    {
        double d0 = super.getMountedYOffset();

        if (this.getType() == HorseArmorType.SKELETON)
        {
            d0 -= 0.1875D;
        }
        else if (this.getType() == HorseArmorType.DONKEY)
        {
            d0 -= 0.25D;
        }

        return d0;
    }

    /**
     * Returns randomized max health
     */
    private float getModifiedMaxHealth()
    {
        return 15.0F + (float)this.rand.nextInt(8) + (float)this.rand.nextInt(9);
    }

    /**
     * Returns randomized jump strength
     */
    private double getModifiedJumpStrength()
    {
        return 0.4000000059604645D + this.rand.nextDouble() * 0.2D + this.rand.nextDouble() * 0.2D + this.rand.nextDouble() * 0.2D;
    }

    /**
     * Returns randomized movement speed
     */
    private double getModifiedMovementSpeed()
    {
        return (0.44999998807907104D + this.rand.nextDouble() * 0.3D + this.rand.nextDouble() * 0.3D + this.rand.nextDouble() * 0.3D) * 0.25D;
    }

    public boolean func_184782_dG()
    {
        return this.field_184793_bU;
    }

    public void func_184784_x(boolean p_184784_1_)
    {
        if (p_184784_1_ != this.field_184793_bU)
        {
            this.field_184793_bU = p_184784_1_;

            if (p_184784_1_)
            {
                this.tasks.addTask(1, this.field_184792_bN);
            }
            else
            {
                this.tasks.removeTask(this.field_184792_bN);
            }
        }
    }

    /**
     * returns true if this entity is by a ladder, false otherwise
     */
    public boolean isOnLadder()
    {
        return false;
    }

    public float getEyeHeight()
    {
        return this.height;
    }

    public boolean replaceItemInInventory(int inventorySlot, ItemStack itemStackIn)
    {
        if (inventorySlot == 499 && this.getType().func_188600_f())
        {
            if (itemStackIn == null && this.isChested())
            {
                this.setChested(false);
                this.initHorseChest();
                return true;
            }

            if (itemStackIn != null && itemStackIn.getItem() == Item.getItemFromBlock(Blocks.chest) && !this.isChested())
            {
                this.setChested(true);
                this.initHorseChest();
                return true;
            }
        }

        int i = inventorySlot - 400;

        if (i >= 0 && i < 2 && i < this.horseChest.getSizeInventory())
        {
            if (i == 0 && itemStackIn != null && itemStackIn.getItem() != Items.saddle)
            {
                return false;
            }
            else if (i != 1 || (itemStackIn == null || HorseType.func_188577_b(itemStackIn.getItem())) && this.getType().func_188603_j())
            {
                this.horseChest.setInventorySlotContents(i, itemStackIn);
                this.updateHorseSlots();
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            int j = inventorySlot - 500 + 2;

            if (j >= 2 && j < this.horseChest.getSizeInventory())
            {
                this.horseChest.setInventorySlotContents(j, itemStackIn);
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * For vehicles, the first passenger is generally considered the controller and "drives" the vehicle. For example,
     * Pigs, Horses, and Boats are generally "steered" by the controlling passenger.
     */
    public Entity getControllingPassenger()
    {
        return this.getPassengers().isEmpty() ? null : (Entity)this.getPassengers().get(0);
    }

    /**
     * Get this Entity's EnumCreatureAttribute
     */
    public EnumCreatureAttribute getCreatureAttribute()
    {
        return this.getType().func_188602_h() ? EnumCreatureAttribute.UNDEAD : EnumCreatureAttribute.UNDEFINED;
    }

    protected ResourceLocation func_184647_J()
    {
        return this.getType().func_188598_l();
    }

    public static class GroupData implements IEntityLivingData
    {
        public HorseArmorType field_188476_a;
        public int field_188477_b;

        public GroupData(HorseArmorType p_i46589_1_, int p_i46589_2_)
        {
            this.field_188476_a = p_i46589_1_;
            this.field_188477_b = p_i46589_2_;
        }
    }
}
