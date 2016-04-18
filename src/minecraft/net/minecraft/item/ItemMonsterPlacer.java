package net.minecraft.item;

import java.util.List;
import java.util.UUID;
import net.minecraft.block.BlockFence;
import net.minecraft.block.BlockLiquid;
import net.minecraft.block.state.IBlockState;
import net.minecraft.creativetab.CreativeTabs;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityList;
import net.minecraft.entity.EntityLiving;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.entity.IEntityLivingData;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.server.MinecraftServer;
import net.minecraft.stats.StatList;
import net.minecraft.tileentity.MobSpawnerBaseLogic;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityMobSpawner;
import net.minecraft.util.ActionResult;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.MathHelper;
import net.minecraft.util.math.RayTraceResult;
import net.minecraft.util.text.translation.I18n;
import net.minecraft.world.World;

public class ItemMonsterPlacer extends Item
{
    public ItemMonsterPlacer()
    {
        this.setCreativeTab(CreativeTabs.tabMisc);
    }

    public String getItemStackDisplayName(ItemStack stack)
    {
        String s = ("" + I18n.translateToLocal(this.getUnlocalizedName() + ".name")).trim();
        String s1 = func_185080_h(stack);

        if (s1 != null)
        {
            s = s + " " + I18n.translateToLocal("entity." + s1 + ".name");
        }

        return s;
    }

    /**
     * Called when a Block is right-clicked with this Item
     */
    public EnumActionResult onItemUse(ItemStack stack, EntityPlayer playerIn, World worldIn, BlockPos pos, EnumHand side, EnumFacing hitX, float hitY, float hitZ, float p_180614_9_)
    {
        if (worldIn.isRemote)
        {
            return EnumActionResult.SUCCESS;
        }
        else if (!playerIn.canPlayerEdit(pos.offset(hitX), hitX, stack))
        {
            return EnumActionResult.FAIL;
        }
        else
        {
            IBlockState iblockstate = worldIn.getBlockState(pos);

            if (iblockstate.getBlock() == Blocks.mob_spawner)
            {
                TileEntity tileentity = worldIn.getTileEntity(pos);

                if (tileentity instanceof TileEntityMobSpawner)
                {
                    MobSpawnerBaseLogic mobspawnerbaselogic = ((TileEntityMobSpawner)tileentity).getSpawnerBaseLogic();
                    mobspawnerbaselogic.setEntityName(func_185080_h(stack));
                    tileentity.markDirty();
                    worldIn.notifyBlockUpdate(pos, iblockstate, iblockstate, 3);

                    if (!playerIn.capabilities.isCreativeMode)
                    {
                        --stack.stackSize;
                    }

                    return EnumActionResult.SUCCESS;
                }
            }

            pos = pos.offset(hitX);
            double d0 = 0.0D;

            if (hitX == EnumFacing.UP && iblockstate instanceof BlockFence)
            {
                d0 = 0.5D;
            }

            Entity entity = spawnCreature(worldIn, func_185080_h(stack), (double)pos.getX() + 0.5D, (double)pos.getY() + d0, (double)pos.getZ() + 0.5D);

            if (entity != null)
            {
                if (entity instanceof EntityLivingBase && stack.hasDisplayName())
                {
                    entity.setCustomNameTag(stack.getDisplayName());
                }

                func_185079_a(worldIn, playerIn, stack, entity);

                if (!playerIn.capabilities.isCreativeMode)
                {
                    --stack.stackSize;
                }
            }

            return EnumActionResult.SUCCESS;
        }
    }

    public static void func_185079_a(World p_185079_0_, EntityPlayer p_185079_1_, ItemStack p_185079_2_, Entity p_185079_3_)
    {
        MinecraftServer minecraftserver = p_185079_0_.getMinecraftServer();

        if (minecraftserver != null && p_185079_3_ != null)
        {
            NBTTagCompound nbttagcompound = p_185079_2_.getTagCompound();

            if (nbttagcompound != null && nbttagcompound.hasKey("EntityTag", 10))
            {
                if (!p_185079_0_.isRemote && p_185079_3_.func_184213_bq() && (p_185079_1_ == null || !minecraftserver.getPlayerList().canSendCommands(p_185079_1_.getGameProfile())))
                {
                    return;
                }

                NBTTagCompound nbttagcompound1 = new NBTTagCompound();
                p_185079_3_.writeToNBT(nbttagcompound1);
                UUID uuid = p_185079_3_.getUniqueID();
                nbttagcompound1.merge(nbttagcompound.getCompoundTag("EntityTag"));
                p_185079_3_.setUniqueId(uuid);
                p_185079_3_.readFromNBT(nbttagcompound1);
            }
        }
    }

    public ActionResult<ItemStack> onItemRightClick(ItemStack itemStackIn, World worldIn, EntityPlayer playerIn, EnumHand hand)
    {
        if (worldIn.isRemote)
        {
            return new ActionResult(EnumActionResult.PASS, itemStackIn);
        }
        else
        {
            RayTraceResult raytraceresult = this.getMovingObjectPositionFromPlayer(worldIn, playerIn, true);

            if (raytraceresult != null && raytraceresult.typeOfHit == RayTraceResult.Type.BLOCK)
            {
                BlockPos blockpos = raytraceresult.getBlockPos();

                if (!(worldIn.getBlockState(blockpos).getBlock() instanceof BlockLiquid))
                {
                    return new ActionResult(EnumActionResult.PASS, itemStackIn);
                }
                else if (worldIn.isBlockModifiable(playerIn, blockpos) && playerIn.canPlayerEdit(blockpos, raytraceresult.sideHit, itemStackIn))
                {
                    Entity entity = spawnCreature(worldIn, func_185080_h(itemStackIn), (double)blockpos.getX() + 0.5D, (double)blockpos.getY() + 0.5D, (double)blockpos.getZ() + 0.5D);

                    if (entity == null)
                    {
                        return new ActionResult(EnumActionResult.PASS, itemStackIn);
                    }
                    else
                    {
                        if (entity instanceof EntityLivingBase && itemStackIn.hasDisplayName())
                        {
                            entity.setCustomNameTag(itemStackIn.getDisplayName());
                        }

                        func_185079_a(worldIn, playerIn, itemStackIn, entity);

                        if (!playerIn.capabilities.isCreativeMode)
                        {
                            --itemStackIn.stackSize;
                        }

                        playerIn.triggerAchievement(StatList.func_188057_b(this));
                        return new ActionResult(EnumActionResult.SUCCESS, itemStackIn);
                    }
                }
                else
                {
                    return new ActionResult(EnumActionResult.FAIL, itemStackIn);
                }
            }
            else
            {
                return new ActionResult(EnumActionResult.PASS, itemStackIn);
            }
        }
    }

    /**
     * Spawns the creature specified by the egg's type in the location specified by the last three parameters.
     * Parameters: world, entityID, x, y, z.
     */
    public static Entity spawnCreature(World worldIn, String entityID, double x, double y, double z)
    {
        if (entityID != null && EntityList.entityEggs.containsKey(entityID))
        {
            Entity entity = null;

            for (int i = 0; i < 1; ++i)
            {
                entity = EntityList.func_188429_b(entityID, worldIn);

                if (entity instanceof EntityLivingBase)
                {
                    EntityLiving entityliving = (EntityLiving)entity;
                    entity.setLocationAndAngles(x, y, z, MathHelper.wrapAngleTo180_float(worldIn.rand.nextFloat() * 360.0F), 0.0F);
                    entityliving.rotationYawHead = entityliving.rotationYaw;
                    entityliving.renderYawOffset = entityliving.rotationYaw;
                    entityliving.onInitialSpawn(worldIn.getDifficultyForLocation(new BlockPos(entityliving)), (IEntityLivingData)null);
                    worldIn.spawnEntityInWorld(entity);
                    entityliving.playLivingSound();
                }
            }

            return entity;
        }
        else
        {
            return null;
        }
    }

    /**
     * returns a list of items with the same ID, but different meta (eg: dye returns 16 items)
     */
    public void getSubItems(Item itemIn, CreativeTabs tab, List<ItemStack> subItems)
    {
        for (EntityList.EntityEggInfo entitylist$entityegginfo : EntityList.entityEggs.values())
        {
            ItemStack itemstack = new ItemStack(itemIn, 1);
            func_185078_a(itemstack, entitylist$entityegginfo.spawnedID);
            subItems.add(itemstack);
        }
    }

    public static void func_185078_a(ItemStack p_185078_0_, String p_185078_1_)
    {
        NBTTagCompound nbttagcompound = p_185078_0_.hasTagCompound() ? p_185078_0_.getTagCompound() : new NBTTagCompound();
        NBTTagCompound nbttagcompound1 = new NBTTagCompound();
        nbttagcompound1.setString("id", p_185078_1_);
        nbttagcompound.setTag("EntityTag", nbttagcompound1);
        p_185078_0_.setTagCompound(nbttagcompound);
    }

    public static String func_185080_h(ItemStack p_185080_0_)
    {
        NBTTagCompound nbttagcompound = p_185080_0_.getTagCompound();

        if (nbttagcompound == null)
        {
            return null;
        }
        else if (!nbttagcompound.hasKey("EntityTag", 10))
        {
            return null;
        }
        else
        {
            NBTTagCompound nbttagcompound1 = nbttagcompound.getCompoundTag("EntityTag");
            return !nbttagcompound1.hasKey("id", 8) ? null : nbttagcompound1.getString("id");
        }
    }
}
