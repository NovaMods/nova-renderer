package net.minecraft.entity.item;

import io.netty.buffer.ByteBuf;
import net.minecraft.block.state.IBlockState;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.init.Blocks;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.network.datasync.DataParameter;
import net.minecraft.network.datasync.DataSerializers;
import net.minecraft.network.datasync.EntityDataManager;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.world.World;

public class EntityMinecartCommandBlock extends EntityMinecart
{
    private static final DataParameter<String> field_184273_a = EntityDataManager.<String>createKey(EntityMinecartCommandBlock.class, DataSerializers.STRING);
    private static final DataParameter<ITextComponent> field_184274_b = EntityDataManager.<ITextComponent>createKey(EntityMinecartCommandBlock.class, DataSerializers.TEXT_COMPONENT);
    private final CommandBlockBaseLogic commandBlockLogic = new CommandBlockBaseLogic()
    {
        public void updateCommand()
        {
            EntityMinecartCommandBlock.this.getDataManager().set(EntityMinecartCommandBlock.field_184273_a, this.getCommand());
            EntityMinecartCommandBlock.this.getDataManager().set(EntityMinecartCommandBlock.field_184274_b, this.getLastOutput());
        }
        public int func_145751_f()
        {
            return 1;
        }
        public void func_145757_a(ByteBuf p_145757_1_)
        {
            p_145757_1_.writeInt(EntityMinecartCommandBlock.this.getEntityId());
        }
        public BlockPos getPosition()
        {
            return new BlockPos(EntityMinecartCommandBlock.this.posX, EntityMinecartCommandBlock.this.posY + 0.5D, EntityMinecartCommandBlock.this.posZ);
        }
        public Vec3d getPositionVector()
        {
            return new Vec3d(EntityMinecartCommandBlock.this.posX, EntityMinecartCommandBlock.this.posY, EntityMinecartCommandBlock.this.posZ);
        }
        public World getEntityWorld()
        {
            return EntityMinecartCommandBlock.this.worldObj;
        }
        public Entity getCommandSenderEntity()
        {
            return EntityMinecartCommandBlock.this;
        }
        public MinecraftServer getServer()
        {
            return EntityMinecartCommandBlock.this.worldObj.getMinecraftServer();
        }
    };

    /** Cooldown before command block logic runs again in ticks */
    private int activatorRailCooldown = 0;

    public EntityMinecartCommandBlock(World p_i46754_1_)
    {
        super(p_i46754_1_);
    }

    public EntityMinecartCommandBlock(World p_i46755_1_, double p_i46755_2_, double p_i46755_4_, double p_i46755_6_)
    {
        super(p_i46755_1_, p_i46755_2_, p_i46755_4_, p_i46755_6_);
    }

    protected void entityInit()
    {
        super.entityInit();
        this.getDataManager().register(field_184273_a, "");
        this.getDataManager().register(field_184274_b, new TextComponentString(""));
    }

    /**
     * (abstract) Protected helper method to read subclass entity data from NBT.
     */
    protected void readEntityFromNBT(NBTTagCompound tagCompund)
    {
        super.readEntityFromNBT(tagCompund);
        this.commandBlockLogic.readDataFromNBT(tagCompund);
        this.getDataManager().set(field_184273_a, this.getCommandBlockLogic().getCommand());
        this.getDataManager().set(field_184274_b, this.getCommandBlockLogic().getLastOutput());
    }

    /**
     * (abstract) Protected helper method to write subclass entity data to NBT.
     */
    protected void writeEntityToNBT(NBTTagCompound tagCompound)
    {
        super.writeEntityToNBT(tagCompound);
        this.commandBlockLogic.writeDataToNBT(tagCompound);
    }

    public EntityMinecart.Type func_184264_v()
    {
        return EntityMinecart.Type.COMMAND_BLOCK;
    }

    public IBlockState getDefaultDisplayTile()
    {
        return Blocks.command_block.getDefaultState();
    }

    public CommandBlockBaseLogic getCommandBlockLogic()
    {
        return this.commandBlockLogic;
    }

    /**
     * Called every tick the minecart is on an activator rail. Args: x, y, z, is the rail receiving power
     */
    public void onActivatorRailPass(int x, int y, int z, boolean receivingPower)
    {
        if (receivingPower && this.ticksExisted - this.activatorRailCooldown >= 4)
        {
            this.getCommandBlockLogic().trigger(this.worldObj);
            this.activatorRailCooldown = this.ticksExisted;
        }
    }

    public boolean func_184230_a(EntityPlayer p_184230_1_, ItemStack p_184230_2_, EnumHand p_184230_3_)
    {
        this.commandBlockLogic.tryOpenEditCommandBlock(p_184230_1_);
        return false;
    }

    public void notifyDataManagerChange(DataParameter<?> key)
    {
        super.notifyDataManagerChange(key);

        if (field_184274_b.equals(key))
        {
            try
            {
                this.commandBlockLogic.setLastOutput((ITextComponent)this.getDataManager().get(field_184274_b));
            }
            catch (Throwable var3)
            {
                ;
            }
        }
        else if (field_184273_a.equals(key))
        {
            this.commandBlockLogic.setCommand((String)this.getDataManager().get(field_184273_a));
        }
    }

    public boolean func_184213_bq()
    {
        return true;
    }
}
