package net.minecraft.entity.item;

import com.google.common.base.Predicate;
import java.util.Arrays;
import java.util.List;
import net.minecraft.block.Block;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.effect.EntityLightningBolt;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.item.Item;
import net.minecraft.item.ItemArmor;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumHand;
import net.minecraft.util.EnumHandSide;
import net.minecraft.util.EnumParticleTypes;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Rotations;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;
import net.minecraft.world.WorldServer;

public class EntityArmorStand extends EntityLivingBase
{
    private static final Rotations DEFAULT_HEAD_ROTATION = new Rotations(0.0F, 0.0F, 0.0F);
    private static final Rotations DEFAULT_BODY_ROTATION = new Rotations(0.0F, 0.0F, 0.0F);
    private static final Rotations DEFAULT_LEFTARM_ROTATION = new Rotations(-10.0F, 0.0F, -10.0F);
    private static final Rotations DEFAULT_RIGHTARM_ROTATION = new Rotations(-15.0F, 0.0F, 10.0F);
    private static final Rotations DEFAULT_LEFTLEG_ROTATION = new Rotations(-1.0F, 0.0F, -1.0F);
    private static final Rotations DEFAULT_RIGHTLEG_ROTATION = new Rotations(1.0F, 0.0F, 1.0F);
    public static final DataParameter<Byte> field_184801_a = EntityDataManager.<Byte>createKey(EntityArmorStand.class, DataSerializers.BYTE);
    public static final DataParameter<Rotations> HEAD_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    public static final DataParameter<Rotations> BODY_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    public static final DataParameter<Rotations> LEFT_ARM_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    public static final DataParameter<Rotations> RIGHT_ARM_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    public static final DataParameter<Rotations> LEFT_LEG_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    public static final DataParameter<Rotations> RIGHT_LEG_ROTATION = EntityDataManager.<Rotations>createKey(EntityArmorStand.class, DataSerializers.ROTATIONS);
    private static final Predicate<Entity> field_184798_bv = new Predicate<Entity>()
    {
        public boolean apply(Entity p_apply_1_)
        {
            return p_apply_1_ instanceof EntityMinecart && ((EntityMinecart)p_apply_1_).func_184264_v() == EntityMinecart.Type.RIDEABLE;
        }
    };
    private final ItemStack[] field_184799_bw;
    private final ItemStack[] field_184800_bx;
    private boolean canInteract;

    /**
     * After punching the stand, the cooldown before you can punch it again without breaking it.
     */
    public long punchCooldown;
    private int disabledSlots;
    private boolean field_181028_bj;
    private Rotations headRotation;
    private Rotations bodyRotation;
    private Rotations leftArmRotation;
    private Rotations rightArmRotation;
    private Rotations leftLegRotation;
    private Rotations rightLegRotation;

    public EntityArmorStand(World worldIn)
    {
        super(worldIn);
        this.field_184799_bw = new ItemStack[2];
        this.field_184800_bx = new ItemStack[4];
        this.headRotation = DEFAULT_HEAD_ROTATION;
        this.bodyRotation = DEFAULT_BODY_ROTATION;
        this.leftArmRotation = DEFAULT_LEFTARM_ROTATION;
        this.rightArmRotation = DEFAULT_RIGHTARM_ROTATION;
        this.leftLegRotation = DEFAULT_LEFTLEG_ROTATION;
        this.rightLegRotation = DEFAULT_RIGHTLEG_ROTATION;
        this.noClip = this.hasNoGravity();
        this.setSize(0.5F, 1.975F);
    }

    public EntityArmorStand(World worldIn, double posX, double posY, double posZ)
    {
        this(worldIn);
        this.setPosition(posX, posY, posZ);
    }

    /**
     * Returns whether the entity is in a server world
     */
    public boolean isServerWorld()
    {
        return super.isServerWorld() && !this.hasNoGravity();
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(field_184801_a, Byte.valueOf((byte)0));
        this.dataWatcher.register(HEAD_ROTATION, DEFAULT_HEAD_ROTATION);
        this.dataWatcher.register(BODY_ROTATION, DEFAULT_BODY_ROTATION);
        this.dataWatcher.register(LEFT_ARM_ROTATION, DEFAULT_LEFTARM_ROTATION);
        this.dataWatcher.register(RIGHT_ARM_ROTATION, DEFAULT_RIGHTARM_ROTATION);
        this.dataWatcher.register(LEFT_LEG_ROTATION, DEFAULT_LEFTLEG_ROTATION);
        this.dataWatcher.register(RIGHT_LEG_ROTATION, DEFAULT_RIGHTLEG_ROTATION);
    }

    public Iterable<ItemStack> getHeldEquipment()
    {
        return Arrays.<ItemStack>asList(this.field_184799_bw);
    }

    public Iterable<ItemStack> getArmorInventoryList()
    {
        return Arrays.<ItemStack>asList(this.field_184800_bx);
    }

    public ItemStack getItemStackFromSlot(EntityEquipmentSlot slotIn)
    {
        ItemStack itemstack = null;

        switch (slotIn.func_188453_a())
        {
            case HAND:
                itemstack = this.field_184799_bw[slotIn.func_188454_b()];
                break;

            case ARMOR:
                itemstack = this.field_184800_bx[slotIn.func_188454_b()];
        }

        return itemstack;
    }

    public void setItemStackToSlot(EntityEquipmentSlot slotIn, ItemStack stack)
    {
        switch (slotIn.func_188453_a())
        {
            case HAND:
                this.func_184606_a_(stack);
                this.field_184799_bw[slotIn.func_188454_b()] = stack;
                break;

            case ARMOR:
                this.func_184606_a_(stack);
                this.field_184800_bx[slotIn.func_188454_b()] = stack;
        }
    }

    public boolean replaceItemInInventory(int inventorySlot, ItemStack itemStackIn)
    {
        EntityEquipmentSlot entityequipmentslot;

        if (inventorySlot == 98)
        {
            entityequipmentslot = EntityEquipmentSlot.MAINHAND;
        }
        else if (inventorySlot == 99)
        {
            entityequipmentslot = EntityEquipmentSlot.OFFHAND;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.HEAD.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.HEAD;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.CHEST.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.CHEST;
        }
        else if (inventorySlot == 100 + EntityEquipmentSlot.LEGS.func_188454_b())
        {
            entityequipmentslot = EntityEquipmentSlot.LEGS;
        }
        else
        {
            if (inventorySlot != 100 + EntityEquipmentSlot.FEET.func_188454_b())
            {
                return false;
            }

            entityequipmentslot = EntityEquipmentSlot.FEET;
        }

        if (itemStackIn != null && !EntityLiving.func_184648_b(entityequipmentslot, itemStackIn) && entityequipmentslot != EntityEquipmentSlot.HEAD)
        {
            return false;
        }
        else
        {
            this.setItemStackToSlot(entityequipmentslot, itemStackIn);
            return true;
        }
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        NBTTagList nbttaglist = new NBTTagList();

        for (int i = 0; i < this.field_184800_bx.length; ++i)
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();

            if (this.field_184800_bx[i] != null)
            {
                this.field_184800_bx[i].writeToNBT(nbttagcompound);
            }

            nbttaglist.appendTag(nbttagcompound);
        }

        tagCompound.setTag("ArmorItems", nbttaglist);
        NBTTagList nbttaglist1 = new NBTTagList();

        for (int j = 0; j < this.field_184799_bw.length; ++j)
        {
            NBTTagCompound nbttagcompound1 = new NBTTagCompound();

            if (this.field_184799_bw[j] != null)
            {
                this.field_184799_bw[j].writeToNBT(nbttagcompound1);
            }

            nbttaglist1.appendTag(nbttagcompound1);
        }

        tagCompound.setTag("HandItems", nbttaglist1);

        if (this.getAlwaysRenderNameTag() && (this.getCustomNameTag() == null || this.getCustomNameTag().isEmpty()))
        {
            tagCompound.setBoolean("CustomNameVisible", this.getAlwaysRenderNameTag());
        }

        tagCompound.setBoolean("Invisible", this.isInvisible());
        tagCompound.setBoolean("Small", this.isSmall());
        tagCompound.setBoolean("ShowArms", this.getShowArms());
        tagCompound.setInteger("DisabledSlots", this.disabledSlots);
        tagCompound.setBoolean("NoGravity", this.hasNoGravity());
        tagCompound.setBoolean("NoBasePlate", this.hasNoBasePlate());

        if (this.hasMarker())
        {
            tagCompound.setBoolean("Marker", this.hasMarker());
        }

        tagCompound.setTag("Pose", this.readPoseFromNBT());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);

        if (tagCompund.hasKey("ArmorItems", 9))
        {
            NBTTagList nbttaglist = tagCompund.getTagList("ArmorItems", 10);

            for (int i = 0; i < this.field_184800_bx.length; ++i)
            {
                this.field_184800_bx[i] = ItemStack.loadItemStackFromNBT(nbttaglist.getCompoundTagAt(i));
            }
        }

        if (tagCompund.hasKey("HandItems", 9))
        {
            NBTTagList nbttaglist1 = tagCompund.getTagList("HandItems", 10);

            for (int j = 0; j < this.field_184799_bw.length; ++j)
            {
                this.field_184799_bw[j] = ItemStack.loadItemStackFromNBT(nbttaglist1.getCompoundTagAt(j));
            }
        }

        this.setInvisible(tagCompund.getBoolean("Invisible"));
        this.setSmall(tagCompund.getBoolean("Small"));
        this.setShowArms(tagCompund.getBoolean("ShowArms"));
        this.disabledSlots = tagCompund.getInteger("DisabledSlots");
        this.setNoGravity(tagCompund.getBoolean("NoGravity"));
        this.setNoBasePlate(tagCompund.getBoolean("NoBasePlate"));
        this.setMarker(tagCompund.getBoolean("Marker"));
        this.field_181028_bj = !this.hasMarker();
        this.noClip = this.hasNoGravity();
        NBTTagCompound nbttagcompound = tagCompund.getCompoundTag("Pose");
        this.writePoseToNBT(nbttagcompound);
    }

    /**
     * Saves the pose to an NBTTagCompound.
     */
    private void writePoseToNBT(NBTTagCompound tagCompound)
    {
        NBTTagList nbttaglist = tagCompound.getTagList("Head", 5);
        this.setHeadRotation(nbttaglist.hasNoTags() ? DEFAULT_HEAD_ROTATION : new Rotations(nbttaglist));
        NBTTagList nbttaglist1 = tagCompound.getTagList("Body", 5);
        this.setBodyRotation(nbttaglist1.hasNoTags() ? DEFAULT_BODY_ROTATION : new Rotations(nbttaglist1));
        NBTTagList nbttaglist2 = tagCompound.getTagList("LeftArm", 5);
        this.setLeftArmRotation(nbttaglist2.hasNoTags() ? DEFAULT_LEFTARM_ROTATION : new Rotations(nbttaglist2));
        NBTTagList nbttaglist3 = tagCompound.getTagList("RightArm", 5);
        this.setRightArmRotation(nbttaglist3.hasNoTags() ? DEFAULT_RIGHTARM_ROTATION : new Rotations(nbttaglist3));
        NBTTagList nbttaglist4 = tagCompound.getTagList("LeftLeg", 5);
        this.setLeftLegRotation(nbttaglist4.hasNoTags() ? DEFAULT_LEFTLEG_ROTATION : new Rotations(nbttaglist4));
        NBTTagList nbttaglist5 = tagCompound.getTagList("RightLeg", 5);
        this.setRightLegRotation(nbttaglist5.hasNoTags() ? DEFAULT_RIGHTLEG_ROTATION : new Rotations(nbttaglist5));
    }

    private NBTTagCompound readPoseFromNBT()
    {
        NBTTagCompound nbttagcompound = new NBTTagCompound();

        if (!DEFAULT_HEAD_ROTATION.equals(this.headRotation))
        {
            nbttagcompound.setTag("Head", this.headRotation.writeToNBT());
        }

        if (!DEFAULT_BODY_ROTATION.equals(this.bodyRotation))
        {
            nbttagcompound.setTag("Body", this.bodyRotation.writeToNBT());
        }

        if (!DEFAULT_LEFTARM_ROTATION.equals(this.leftArmRotation))
        {
            nbttagcompound.setTag("LeftArm", this.leftArmRotation.writeToNBT());
        }

        if (!DEFAULT_RIGHTARM_ROTATION.equals(this.rightArmRotation))
        {
            nbttagcompound.setTag("RightArm", this.rightArmRotation.writeToNBT());
        }

        if (!DEFAULT_LEFTLEG_ROTATION.equals(this.leftLegRotation))
        {
            nbttagcompound.setTag("LeftLeg", this.leftLegRotation.writeToNBT());
        }

        if (!DEFAULT_RIGHTLEG_ROTATION.equals(this.rightLegRotation))
        {
            nbttagcompound.setTag("RightLeg", this.rightLegRotation.writeToNBT());
        }

        return nbttagcompound;
    }

    /**
     * Returns true if this entity should push and be pushed by other entities when colliding.
     */
    public boolean canBePushed()
    {
        return false;
    }

    protected void collideWithEntity(Entity entityIn)
    {
    }

    protected void collideWithNearbyEntities()
    {
        List<Entity> list = this.worldObj.getEntitiesInAABBexcluding(this, this.getEntityBoundingBox(), field_184798_bv);

        for (int i = 0; i < list.size(); ++i)
        {
            Entity entity = (Entity)list.get(i);

            if (this.getDistanceSqToEntity(entity) <= 0.2D)
            {
                entity.applyEntityCollision(this);
            }
        }
    }

    public EnumActionResult func_184199_a(EntityPlayer player, Vec3d vec, ItemStack stack, EnumHand hand)
    {
        if (this.hasMarker())
        {
            return EnumActionResult.PASS;
        }
        else if (!this.worldObj.isRemote && !player.isSpectator())
        {
            EntityEquipmentSlot entityequipmentslot = EntityEquipmentSlot.MAINHAND;
            boolean flag = stack != null;
            Item item = flag ? stack.getItem() : null;

            if (flag && item instanceof ItemArmor)
            {
                entityequipmentslot = ((ItemArmor)item).armorType;
            }

            if (flag && (item == Items.skull || item == Item.getItemFromBlock(Blocks.pumpkin)))
            {
                entityequipmentslot = EntityEquipmentSlot.HEAD;
            }

            double d0 = 0.1D;
            double d1 = 0.9D;
            double d2 = 0.4D;
            double d3 = 1.6D;
            EntityEquipmentSlot entityequipmentslot1 = EntityEquipmentSlot.MAINHAND;
            boolean flag1 = this.isSmall();
            double d4 = flag1 ? vec.yCoord * 2.0D : vec.yCoord;

            if (d4 >= 0.1D && d4 < 0.1D + (flag1 ? 0.8D : 0.45D) && this.getItemStackFromSlot(EntityEquipmentSlot.FEET) != null)
            {
                entityequipmentslot1 = EntityEquipmentSlot.FEET;
            }
            else if (d4 >= 0.9D + (flag1 ? 0.3D : 0.0D) && d4 < 0.9D + (flag1 ? 1.0D : 0.7D) && this.getItemStackFromSlot(EntityEquipmentSlot.CHEST) != null)
            {
                entityequipmentslot1 = EntityEquipmentSlot.CHEST;
            }
            else if (d4 >= 0.4D && d4 < 0.4D + (flag1 ? 1.0D : 0.8D) && this.getItemStackFromSlot(EntityEquipmentSlot.LEGS) != null)
            {
                entityequipmentslot1 = EntityEquipmentSlot.LEGS;
            }
            else if (d4 >= 1.6D && this.getItemStackFromSlot(EntityEquipmentSlot.HEAD) != null)
            {
                entityequipmentslot1 = EntityEquipmentSlot.HEAD;
            }

            boolean flag2 = this.getItemStackFromSlot(entityequipmentslot1) != null;

            if (this.func_184796_b(entityequipmentslot1) || this.func_184796_b(entityequipmentslot))
            {
                entityequipmentslot1 = entityequipmentslot;

                if (this.func_184796_b(entityequipmentslot))
                {
                    return EnumActionResult.FAIL;
                }
            }

            if (flag && entityequipmentslot == EntityEquipmentSlot.MAINHAND && !this.getShowArms())
            {
                return EnumActionResult.FAIL;
            }
            else
            {
                if (flag)
                {
                    this.func_184795_a(player, entityequipmentslot, stack, hand);
                }
                else if (flag2)
                {
                    this.func_184795_a(player, entityequipmentslot1, stack, hand);
                }

                return EnumActionResult.SUCCESS;
            }
        }
        else
        {
            return EnumActionResult.SUCCESS;
        }
    }

    private boolean func_184796_b(EntityEquipmentSlot p_184796_1_)
    {
        return (this.disabledSlots & 1 << p_184796_1_.func_188452_c()) != 0;
    }

    private void func_184795_a(EntityPlayer p_184795_1_, EntityEquipmentSlot p_184795_2_, ItemStack p_184795_3_, EnumHand p_184795_4_)
    {
        ItemStack itemstack = this.getItemStackFromSlot(p_184795_2_);

        if (itemstack == null || (this.disabledSlots & 1 << p_184795_2_.func_188452_c() + 8) == 0)
        {
            if (itemstack != null || (this.disabledSlots & 1 << p_184795_2_.func_188452_c() + 16) == 0)
            {
                if (p_184795_1_.capabilities.isCreativeMode && (itemstack == null || itemstack.getItem() == Item.getItemFromBlock(Blocks.air)) && p_184795_3_ != null)
                {
                    ItemStack itemstack2 = p_184795_3_.copy();
                    itemstack2.stackSize = 1;
                    this.setItemStackToSlot(p_184795_2_, itemstack2);
                }
                else if (p_184795_3_ != null && p_184795_3_.stackSize > 1)
                {
                    if (itemstack == null)
                    {
                        ItemStack itemstack1 = p_184795_3_.copy();
                        itemstack1.stackSize = 1;
                        this.setItemStackToSlot(p_184795_2_, itemstack1);
                        --p_184795_3_.stackSize;
                    }
                }
                else
                {
                    this.setItemStackToSlot(p_184795_2_, p_184795_3_);
                    p_184795_1_.setHeldItem(p_184795_4_, itemstack);
                }
            }
        }
    }

    /**
     * Called when the entity is attacked.
     */
    public boolean attackEntityFrom(DamageSource source, float amount)
    {
        if (!this.worldObj.isRemote && !this.isDead)
        {
            if (DamageSource.outOfWorld.equals(source))
            {
                this.setDead();
                return false;
            }
            else if (!this.isEntityInvulnerable(source) && !this.canInteract && !this.hasMarker())
            {
                if (source.isExplosion())
                {
                    this.dropContents();
                    this.setDead();
                    return false;
                }
                else if (DamageSource.inFire.equals(source))
                {
                    if (this.isBurning())
                    {
                        this.damageArmorStand(0.15F);
                    }
                    else
                    {
                        this.setFire(5);
                    }

                    return false;
                }
                else if (DamageSource.onFire.equals(source) && this.getHealth() > 0.5F)
                {
                    this.damageArmorStand(4.0F);
                    return false;
                }
                else
                {
                    boolean flag = "arrow".equals(source.getDamageType());
                    boolean flag1 = "player".equals(source.getDamageType());

                    if (!flag1 && !flag)
                    {
                        return false;
                    }
                    else
                    {
                        if (source.getSourceOfDamage() instanceof EntityArrow)
                        {
                            source.getSourceOfDamage().setDead();
                        }

                        if (source.getEntity() instanceof EntityPlayer && !((EntityPlayer)source.getEntity()).capabilities.allowEdit)
                        {
                            return false;
                        }
                        else if (source.isCreativePlayer())
                        {
                            this.playParticles();
                            this.setDead();
                            return false;
                        }
                        else
                        {
                            long i = this.worldObj.getTotalWorldTime();

                            if (i - this.punchCooldown > 5L && !flag)
                            {
                                this.worldObj.setEntityState(this, (byte)32);
                                this.punchCooldown = i;
                            }
                            else
                            {
                                this.dropBlock();
                                this.playParticles();
                                this.setDead();
                            }

                            return false;
                        }
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 32)
        {
            if (this.worldObj.isRemote)
            {
                this.worldObj.func_184134_a(this.posX, this.posY, this.posZ, SoundEvents.entity_armorstand_hit, this.getSoundCategory(), 0.3F, 1.0F, false);
                this.punchCooldown = this.worldObj.getTotalWorldTime();
            }
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    /**
     * Checks if the entity is in range to render by using the past in distance and comparing it to its average edge
     * length * 64 * renderDistanceWeight Args: distance
     */
    public boolean isInRangeToRenderDist(double distance)
    {
        double d0 = this.getEntityBoundingBox().getAverageEdgeLength() * 4.0D;

        if (Double.isNaN(d0) || d0 == 0.0D)
        {
            d0 = 4.0D;
        }

        d0 = d0 * 64.0D;
        return distance < d0 * d0;
    }

    private void playParticles()
    {
        if (this.worldObj instanceof WorldServer)
        {
            ((WorldServer)this.worldObj).spawnParticle(EnumParticleTypes.BLOCK_DUST, this.posX, this.posY + (double)this.height / 1.5D, this.posZ, 10, (double)(this.width / 4.0F), (double)(this.height / 4.0F), (double)(this.width / 4.0F), 0.05D, new int[] {Block.getStateId(Blocks.planks.getDefaultState())});
        }
    }

    private void damageArmorStand(float p_175406_1_)
    {
        float f = this.getHealth();
        f = f - p_175406_1_;

        if (f <= 0.5F)
        {
            this.dropContents();
            this.setDead();
        }
        else
        {
            this.setHealth(f);
        }
    }

    private void dropBlock()
    {
        Block.spawnAsEntity(this.worldObj, new BlockPos(this), new ItemStack(Items.armor_stand));
        this.dropContents();
    }

    private void dropContents()
    {
        this.worldObj.func_184148_a((EntityPlayer)null, this.posX, this.posY, this.posZ, SoundEvents.entity_armorstand_break, this.getSoundCategory(), 1.0F, 1.0F);

        for (int i = 0; i < this.field_184799_bw.length; ++i)
        {
            if (this.field_184799_bw[i] != null && this.field_184799_bw[i].stackSize > 0)
            {
                if (this.field_184799_bw[i] != null)
                {
                    Block.spawnAsEntity(this.worldObj, (new BlockPos(this)).up(), this.field_184799_bw[i]);
                }

                this.field_184799_bw[i] = null;
            }
        }

        for (int j = 0; j < this.field_184800_bx.length; ++j)
        {
            if (this.field_184800_bx[j] != null && this.field_184800_bx[j].stackSize > 0)
            {
                if (this.field_184800_bx[j] != null)
                {
                    Block.spawnAsEntity(this.worldObj, (new BlockPos(this)).up(), this.field_184800_bx[j]);
                }

                this.field_184800_bx[j] = null;
            }
        }
    }

    protected float updateDistance(float p_110146_1_, float p_110146_2_)
    {
        this.prevRenderYawOffset = this.prevRotationYaw;
        this.renderYawOffset = this.rotationYaw;
        return 0.0F;
    }

    public float getEyeHeight()
    {
        return this.isChild() ? this.height * 0.5F : this.height * 0.9F;
    }

    /**
     * Returns the Y Offset of this entity.
     */
    public double getYOffset()
    {
        return this.hasMarker() ? 0.0D : 0.10000000149011612D;
    }

    /**
     * Moves the entity based on the specified heading.  Args: strafe, forward
     */
    public void moveEntityWithHeading(float strafe, float forward)
    {
        if (!this.hasNoGravity())
        {
            super.moveEntityWithHeading(strafe, forward);
        }
    }

    /**
     * Called to update the entity's position/logic.
     */
    public void onUpdate()
    {
        super.onUpdate();
        Rotations rotations = (Rotations)this.dataWatcher.get(HEAD_ROTATION);

        if (!this.headRotation.equals(rotations))
        {
            this.setHeadRotation(rotations);
        }

        Rotations rotations1 = (Rotations)this.dataWatcher.get(BODY_ROTATION);

        if (!this.bodyRotation.equals(rotations1))
        {
            this.setBodyRotation(rotations1);
        }

        Rotations rotations2 = (Rotations)this.dataWatcher.get(LEFT_ARM_ROTATION);

        if (!this.leftArmRotation.equals(rotations2))
        {
            this.setLeftArmRotation(rotations2);
        }

        Rotations rotations3 = (Rotations)this.dataWatcher.get(RIGHT_ARM_ROTATION);

        if (!this.rightArmRotation.equals(rotations3))
        {
            this.setRightArmRotation(rotations3);
        }

        Rotations rotations4 = (Rotations)this.dataWatcher.get(LEFT_LEG_ROTATION);

        if (!this.leftLegRotation.equals(rotations4))
        {
            this.setLeftLegRotation(rotations4);
        }

        Rotations rotations5 = (Rotations)this.dataWatcher.get(RIGHT_LEG_ROTATION);

        if (!this.rightLegRotation.equals(rotations5))
        {
            this.setRightLegRotation(rotations5);
        }

        boolean flag = this.hasMarker();

        if (!this.field_181028_bj && flag)
        {
            this.func_181550_a(false);
            this.preventEntitySpawning = false;
        }
        else
        {
            if (!this.field_181028_bj || flag)
            {
                return;
            }

            this.func_181550_a(true);
            this.preventEntitySpawning = true;
        }

        this.field_181028_bj = flag;
    }

    private void func_181550_a(boolean p_181550_1_)
    {
        double d0 = this.posX;
        double d1 = this.posY;
        double d2 = this.posZ;

        if (p_181550_1_)
        {
            this.setSize(0.5F, 1.975F);
        }
        else
        {
            this.setSize(0.0F, 0.0F);
        }

        this.setPosition(d0, d1, d2);
    }

    /**
     * Clears potion metadata values if the entity has no potion effects. Otherwise, updates potion effect color,
     * ambience, and invisibility metadata values
     */
    protected void updatePotionMetadata()
    {
        this.setInvisible(this.canInteract);
    }

    public void setInvisible(boolean invisible)
    {
        this.canInteract = invisible;
        super.setInvisible(invisible);
    }

    /**
     * If Animal, checks if the age timer is negative
     */
    public boolean isChild()
    {
        return this.isSmall();
    }

    /**
     * Called by the /kill command.
     */
    public void onKillCommand()
    {
        this.setDead();
    }

    public boolean isImmuneToExplosions()
    {
        return this.isInvisible();
    }

    private void setSmall(boolean p_175420_1_)
    {
        this.dataWatcher.set(field_184801_a, Byte.valueOf(this.func_184797_a(((Byte)this.dataWatcher.get(field_184801_a)).byteValue(), 1, p_175420_1_)));
    }

    public boolean isSmall()
    {
        return (((Byte)this.dataWatcher.get(field_184801_a)).byteValue() & 1) != 0;
    }

    private void setNoGravity(boolean p_175425_1_)
    {
        this.dataWatcher.set(field_184801_a, Byte.valueOf(this.func_184797_a(((Byte)this.dataWatcher.get(field_184801_a)).byteValue(), 2, p_175425_1_)));
    }

    public boolean hasNoGravity()
    {
        return (((Byte)this.dataWatcher.get(field_184801_a)).byteValue() & 2) != 0;
    }

    private void setShowArms(boolean p_175413_1_)
    {
        this.dataWatcher.set(field_184801_a, Byte.valueOf(this.func_184797_a(((Byte)this.dataWatcher.get(field_184801_a)).byteValue(), 4, p_175413_1_)));
    }

    public boolean getShowArms()
    {
        return (((Byte)this.dataWatcher.get(field_184801_a)).byteValue() & 4) != 0;
    }

    private void setNoBasePlate(boolean p_175426_1_)
    {
        this.dataWatcher.set(field_184801_a, Byte.valueOf(this.func_184797_a(((Byte)this.dataWatcher.get(field_184801_a)).byteValue(), 8, p_175426_1_)));
    }

    public boolean hasNoBasePlate()
    {
        return (((Byte)this.dataWatcher.get(field_184801_a)).byteValue() & 8) != 0;
    }

    /**
     * Marker defines where if true, the size is 0 and will not be rendered or intractable.
     */
    private void setMarker(boolean p_181027_1_)
    {
        this.dataWatcher.set(field_184801_a, Byte.valueOf(this.func_184797_a(((Byte)this.dataWatcher.get(field_184801_a)).byteValue(), 16, p_181027_1_)));
    }

    /**
     * Gets whether the armor stand has marker enabled. If true, the armor stand's bounding box is set to zero and
     * cannot be interacted with.
     */
    public boolean hasMarker()
    {
        return (((Byte)this.dataWatcher.get(field_184801_a)).byteValue() & 16) != 0;
    }

    private byte func_184797_a(byte p_184797_1_, int p_184797_2_, boolean p_184797_3_)
    {
        if (p_184797_3_)
        {
            p_184797_1_ = (byte)(p_184797_1_ | p_184797_2_);
        }
        else
        {
            p_184797_1_ = (byte)(p_184797_1_ & ~p_184797_2_);
        }

        return p_184797_1_;
    }

    public void setHeadRotation(Rotations p_175415_1_)
    {
        this.headRotation = p_175415_1_;
        this.dataWatcher.set(HEAD_ROTATION, p_175415_1_);
    }

    public void setBodyRotation(Rotations p_175424_1_)
    {
        this.bodyRotation = p_175424_1_;
        this.dataWatcher.set(BODY_ROTATION, p_175424_1_);
    }

    public void setLeftArmRotation(Rotations p_175405_1_)
    {
        this.leftArmRotation = p_175405_1_;
        this.dataWatcher.set(LEFT_ARM_ROTATION, p_175405_1_);
    }

    public void setRightArmRotation(Rotations p_175428_1_)
    {
        this.rightArmRotation = p_175428_1_;
        this.dataWatcher.set(RIGHT_ARM_ROTATION, p_175428_1_);
    }

    public void setLeftLegRotation(Rotations p_175417_1_)
    {
        this.leftLegRotation = p_175417_1_;
        this.dataWatcher.set(LEFT_LEG_ROTATION, p_175417_1_);
    }

    public void setRightLegRotation(Rotations p_175427_1_)
    {
        this.rightLegRotation = p_175427_1_;
        this.dataWatcher.set(RIGHT_LEG_ROTATION, p_175427_1_);
    }

    public Rotations getHeadRotation()
    {
        return this.headRotation;
    }

    public Rotations getBodyRotation()
    {
        return this.bodyRotation;
    }

    public Rotations getLeftArmRotation()
    {
        return this.leftArmRotation;
    }

    public Rotations getRightArmRotation()
    {
        return this.rightArmRotation;
    }

    public Rotations getLeftLegRotation()
    {
        return this.leftLegRotation;
    }

    public Rotations getRightLegRotation()
    {
        return this.rightLegRotation;
    }

    /**
     * Returns true if other Entities should be prevented from moving through this Entity.
     */
    public boolean canBeCollidedWith()
    {
        return super.canBeCollidedWith() && !this.hasMarker();
    }

    public EnumHandSide getPrimaryHand()
    {
        return EnumHandSide.RIGHT;
    }

    protected SoundEvent getFallSound(int heightIn)
    {
        return SoundEvents.entity_armorstand_fall;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_armorstand_hit;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_armorstand_break;
    }

    /**
     * Called when a lightning bolt hits the entity.
     */
    public void onStruckByLightning(EntityLightningBolt lightningBolt)
    {
    }

    public boolean func_184603_cC()
    {
        return false;
    }
}
