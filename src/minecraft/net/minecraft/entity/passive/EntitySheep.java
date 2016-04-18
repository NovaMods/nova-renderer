package net.minecraft.entity.passive;

import com.google.common.collect.Maps;
import java.util.Map;
import java.util.Random;
import net.minecraft.block.Block;
import net.minecraft.entity.EntityAgeable;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.SharedMonsterAttributes;
import net.minecraft.entity.ai.EntityAIEatGrass;
import net.minecraft.entity.ai.EntityAIFollowParent;
import net.minecraft.entity.ai.EntityAILookIdle;
import net.minecraft.entity.ai.EntityAIMate;
import net.minecraft.entity.ai.EntityAIPanic;
import net.minecraft.entity.ai.EntityAISwimming;
import net.minecraft.entity.ai.EntityAITempt;
import net.minecraft.entity.ai.EntityAIWander;
import net.minecraft.entity.ai.EntityAIWatchClosest;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.SoundEvents;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.InventoryCrafting;
import net.minecraft.item.EnumDyeColor;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.item.crafting.CraftingManager;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.SoundEvent;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.DifficultyInstance;
import net.minecraft.world.World;
import net.minecraft.world.storage.loot.LootTableList;

public class EntitySheep extends EntityAnimal
{
    private static final DataParameter<Byte> DYE_COLOR = EntityDataManager.<Byte>createKey(EntitySheep.class, DataSerializers.BYTE);

    /**
     * Internal crafting inventory used to check the result of mixing dyes corresponding to the fleece color when
     * breeding sheep.
     */
    private final InventoryCrafting inventoryCrafting = new InventoryCrafting(new Container()
    {
        public boolean canInteractWith(EntityPlayer playerIn)
        {
            return false;
        }
    }, 2, 1);
    private static final Map<EnumDyeColor, float[]> DYE_TO_RGB = Maps.newEnumMap(EnumDyeColor.class);

    /**
     * Used to control movement as well as wool regrowth. Set to 40 on handleHealthUpdate and counts down with each
     * tick.
     */
    private int sheepTimer;
    private EntityAIEatGrass entityAIEatGrass;

    public static float[] getDyeRgb(EnumDyeColor dyeColor)
    {
        return (float[])DYE_TO_RGB.get(dyeColor);
    }

    public EntitySheep(World worldIn)
    {
        super(worldIn);
        this.setSize(0.9F, 1.3F);
        this.inventoryCrafting.setInventorySlotContents(0, new ItemStack(Items.dye));
        this.inventoryCrafting.setInventorySlotContents(1, new ItemStack(Items.dye));
    }

    protected void initEntityAI()
    {
        this.tasks.addTask(0, new EntityAISwimming(this));
        this.tasks.addTask(1, new EntityAIPanic(this, 1.25D));
        this.tasks.addTask(2, new EntityAIMate(this, 1.0D));
        this.tasks.addTask(3, new EntityAITempt(this, 1.1D, Items.wheat, false));
        this.tasks.addTask(4, new EntityAIFollowParent(this, 1.1D));
        this.tasks.addTask(5, this.entityAIEatGrass = new EntityAIEatGrass(this));
        this.tasks.addTask(6, new EntityAIWander(this, 1.0D));
        this.tasks.addTask(7, new EntityAIWatchClosest(this, EntityPlayer.class, 6.0F));
        this.tasks.addTask(8, new EntityAILookIdle(this));
    }

    protected void updateAITasks()
    {
        this.sheepTimer = this.entityAIEatGrass.getEatingGrassTimer();
        super.updateAITasks();
    }

    /**
     * Called frequently so the entity can update its state every tick as required. For example, zombies and skeletons
     * use this to react to sunlight and start to burn.
     */
    public void onLivingUpdate()
    {
        if (this.worldObj.isRemote)
        {
            this.sheepTimer = Math.max(0, this.sheepTimer - 1);
        }

        super.onLivingUpdate();
    }

    protected void applyEntityAttributes()
    {
        super.applyEntityAttributes();
        this.getEntityAttribute(SharedMonsterAttributes.MAX_HEALTH).setBaseValue(8.0D);
        this.getEntityAttribute(SharedMonsterAttributes.MOVEMENT_SPEED).setBaseValue(0.23000000417232513D);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.dataWatcher.register(DYE_COLOR, Byte.valueOf((byte)0));
    }

    protected ResourceLocation func_184647_J()
    {
        if (this.getSheared())
        {
            return LootTableList.ENTITIES_SHEEP;
        }
        else
        {
            switch (this.getFleeceColor())
            {
                case WHITE:
                default:
                    return LootTableList.ENTITIES_SHEEP_WHITE;

                case ORANGE:
                    return LootTableList.ENTITIES_SHEEP_ORANGE;

                case MAGENTA:
                    return LootTableList.ENTITIES_SHEEP_MAGENTA;

                case LIGHT_BLUE:
                    return LootTableList.ENTITIES_SHEEP_LIGHT_BLUE;

                case YELLOW:
                    return LootTableList.ENTITIES_SHEEP_YELLOW;

                case LIME:
                    return LootTableList.ENTITIES_SHEEP_LIME;

                case PINK:
                    return LootTableList.ENTITIES_SHEEP_PINK;

                case GRAY:
                    return LootTableList.ENTITIES_SHEEP_GRAY;

                case SILVER:
                    return LootTableList.ENTITIES_SHEEP_SILVER;

                case CYAN:
                    return LootTableList.ENTITIES_SHEEP_CYAN;

                case PURPLE:
                    return LootTableList.ENTITIES_SHEEP_PURPLE;

                case BLUE:
                    return LootTableList.ENTITIES_SHEEP_BLUE;

                case BROWN:
                    return LootTableList.ENTITIES_SHEEP_BROWN;

                case GREEN:
                    return LootTableList.ENTITIES_SHEEP_GREEN;

                case RED:
                    return LootTableList.ENTITIES_SHEEP_RED;

                case BLACK:
                    return LootTableList.ENTITIES_SHEEP_BLACK;
            }
        }
    }

    public void handleStatusUpdate(byte id)
    {
        if (id == 10)
        {
            this.sheepTimer = 40;
        }
        else
        {
            super.handleStatusUpdate(id);
        }
    }

    public float getHeadRotationPointY(float p_70894_1_)
    {
        return this.sheepTimer <= 0 ? 0.0F : (this.sheepTimer >= 4 && this.sheepTimer <= 36 ? 1.0F : (this.sheepTimer < 4 ? ((float)this.sheepTimer - p_70894_1_) / 4.0F : -((float)(this.sheepTimer - 40) - p_70894_1_) / 4.0F));
    }

    public float getHeadRotationAngleX(float p_70890_1_)
    {
        if (this.sheepTimer > 4 && this.sheepTimer <= 36)
        {
            float f = ((float)(this.sheepTimer - 4) - p_70890_1_) / 32.0F;
            return ((float)Math.PI / 5F) + ((float)Math.PI * 7F / 100F) * MathHelper.sin(f * 28.7F);
        }
        else
        {
            return this.sheepTimer > 0 ? ((float)Math.PI / 5F) : this.rotationPitch * 0.017453292F;
        }
    }

    public boolean func_184645_a(EntityPlayer p_184645_1_, EnumHand p_184645_2_, ItemStack p_184645_3_)
    {
        if (p_184645_3_ != null && p_184645_3_.getItem() == Items.shears && !this.getSheared() && !this.isChild())
        {
            if (!this.worldObj.isRemote)
            {
                this.setSheared(true);
                int i = 1 + this.rand.nextInt(3);

                for (int j = 0; j < i; ++j)
                {
                    EntityItem entityitem = this.entityDropItem(new ItemStack(Item.getItemFromBlock(Blocks.wool), 1, this.getFleeceColor().getMetadata()), 1.0F);
                    entityitem.motionY += (double)(this.rand.nextFloat() * 0.05F);
                    entityitem.motionX += (double)((this.rand.nextFloat() - this.rand.nextFloat()) * 0.1F);
                    entityitem.motionZ += (double)((this.rand.nextFloat() - this.rand.nextFloat()) * 0.1F);
                }
            }

            p_184645_3_.damageItem(1, p_184645_1_);
            this.playSound(SoundEvents.entity_sheep_shear, 1.0F, 1.0F);
        }

        return super.func_184645_a(p_184645_1_, p_184645_2_, p_184645_3_);
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    public void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        tagCompound.setBoolean("Sheared", this.getSheared());
        tagCompound.setByte("Color", (byte)this.getFleeceColor().getMetadata());
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    public void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.setSheared(tagCompund.getBoolean("Sheared"));
        this.setFleeceColor(EnumDyeColor.byMetadata(tagCompund.getByte("Color")));
    }

    protected SoundEvent getAmbientSound()
    {
        return SoundEvents.entity_sheep_ambient;
    }

    protected SoundEvent getHurtSound()
    {
        return SoundEvents.entity_sheep_hurt;
    }

    protected SoundEvent getDeathSound()
    {
        return SoundEvents.entity_sheep_death;
    }

    protected void playStepSound(BlockPos pos, Block blockIn)
    {
        this.playSound(SoundEvents.entity_sheep_step, 0.15F, 1.0F);
    }

    /**
     * Gets the wool color of this sheep.
     */
    public EnumDyeColor getFleeceColor()
    {
        return EnumDyeColor.byMetadata(((Byte)this.dataWatcher.get(DYE_COLOR)).byteValue() & 15);
    }

    /**
     * Sets the wool color of this sheep
     */
    public void setFleeceColor(EnumDyeColor color)
    {
        byte b0 = ((Byte)this.dataWatcher.get(DYE_COLOR)).byteValue();
        this.dataWatcher.set(DYE_COLOR, Byte.valueOf((byte)(b0 & 240 | color.getMetadata() & 15)));
    }

    /**
     * returns true if a sheeps wool has been sheared
     */
    public boolean getSheared()
    {
        return (((Byte)this.dataWatcher.get(DYE_COLOR)).byteValue() & 16) != 0;
    }

    /**
     * make a sheep sheared if set to true
     */
    public void setSheared(boolean sheared)
    {
        byte b0 = ((Byte)this.dataWatcher.get(DYE_COLOR)).byteValue();

        if (sheared)
        {
            this.dataWatcher.set(DYE_COLOR, Byte.valueOf((byte)(b0 | 16)));
        }
        else
        {
            this.dataWatcher.set(DYE_COLOR, Byte.valueOf((byte)(b0 & -17)));
        }
    }

    /**
     * Chooses a "vanilla" sheep color based on the provided random.
     */
    public static EnumDyeColor getRandomSheepColor(Random random)
    {
        int i = random.nextInt(100);
        return i < 5 ? EnumDyeColor.BLACK : (i < 10 ? EnumDyeColor.GRAY : (i < 15 ? EnumDyeColor.SILVER : (i < 18 ? EnumDyeColor.BROWN : (random.nextInt(500) == 0 ? EnumDyeColor.PINK : EnumDyeColor.WHITE))));
    }

    public EntitySheep createChild(EntityAgeable ageable)
    {
        EntitySheep entitysheep = (EntitySheep)ageable;
        EntitySheep entitysheep1 = new EntitySheep(this.worldObj);
        entitysheep1.setFleeceColor(this.getDyeColorMixFromParents(this, entitysheep));
        return entitysheep1;
    }

    /**
     * This function applies the benefits of growing back wool and faster growing up to the acting entity. (This
     * function is used in the AIEatGrass)
     */
    public void eatGrassBonus()
    {
        this.setSheared(false);

        if (this.isChild())
        {
            this.addGrowth(60);
        }
    }

    /**
     * Called only once on an entity when first time spawned, via egg, mob spawner, natural spawning etc, but not called
     * when entity is reloaded from nbt. Mainly used for initializing attributes and inventory
     */
    public IEntityLivingData onInitialSpawn(DifficultyInstance difficulty, IEntityLivingData livingdata)
    {
        livingdata = super.onInitialSpawn(difficulty, livingdata);
        this.setFleeceColor(getRandomSheepColor(this.worldObj.rand));
        return livingdata;
    }

    /**
     * Attempts to mix both parent sheep to come up with a mixed dye color.
     */
    private EnumDyeColor getDyeColorMixFromParents(EntityAnimal father, EntityAnimal mother)
    {
        int i = ((EntitySheep)father).getFleeceColor().getDyeDamage();
        int j = ((EntitySheep)mother).getFleeceColor().getDyeDamage();
        this.inventoryCrafting.getStackInSlot(0).setItemDamage(i);
        this.inventoryCrafting.getStackInSlot(1).setItemDamage(j);
        ItemStack itemstack = CraftingManager.getInstance().findMatchingRecipe(this.inventoryCrafting, ((EntitySheep)father).worldObj);
        int k;

        if (itemstack != null && itemstack.getItem() == Items.dye)
        {
            k = itemstack.getMetadata();
        }
        else
        {
            k = this.worldObj.rand.nextBoolean() ? i : j;
        }

        return EnumDyeColor.byDyeDamage(k);
    }

    public float getEyeHeight()
    {
        return 0.95F * this.height;
    }

    static
    {
        DYE_TO_RGB.put(EnumDyeColor.WHITE, new float[] {1.0F, 1.0F, 1.0F});
        DYE_TO_RGB.put(EnumDyeColor.ORANGE, new float[] {0.85F, 0.5F, 0.2F});
        DYE_TO_RGB.put(EnumDyeColor.MAGENTA, new float[] {0.7F, 0.3F, 0.85F});
        DYE_TO_RGB.put(EnumDyeColor.LIGHT_BLUE, new float[] {0.4F, 0.6F, 0.85F});
        DYE_TO_RGB.put(EnumDyeColor.YELLOW, new float[] {0.9F, 0.9F, 0.2F});
        DYE_TO_RGB.put(EnumDyeColor.LIME, new float[] {0.5F, 0.8F, 0.1F});
        DYE_TO_RGB.put(EnumDyeColor.PINK, new float[] {0.95F, 0.5F, 0.65F});
        DYE_TO_RGB.put(EnumDyeColor.GRAY, new float[] {0.3F, 0.3F, 0.3F});
        DYE_TO_RGB.put(EnumDyeColor.SILVER, new float[] {0.6F, 0.6F, 0.6F});
        DYE_TO_RGB.put(EnumDyeColor.CYAN, new float[] {0.3F, 0.5F, 0.6F});
        DYE_TO_RGB.put(EnumDyeColor.PURPLE, new float[] {0.5F, 0.25F, 0.7F});
        DYE_TO_RGB.put(EnumDyeColor.BLUE, new float[] {0.2F, 0.3F, 0.7F});
        DYE_TO_RGB.put(EnumDyeColor.BROWN, new float[] {0.4F, 0.3F, 0.2F});
        DYE_TO_RGB.put(EnumDyeColor.GREEN, new float[] {0.4F, 0.5F, 0.2F});
        DYE_TO_RGB.put(EnumDyeColor.RED, new float[] {0.6F, 0.2F, 0.2F});
        DYE_TO_RGB.put(EnumDyeColor.BLACK, new float[] {0.1F, 0.1F, 0.1F});
    }
}
