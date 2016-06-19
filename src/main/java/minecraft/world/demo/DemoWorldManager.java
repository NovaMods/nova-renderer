package net.minecraft.world.demo;

import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.item.ItemStack;
import net.minecraft.network.play.server.SPacketChangeGameState;
import net.minecraft.server.management.PlayerInteractionManager;
import net.minecraft.util.EnumActionResult;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.world.World;

public class DemoWorldManager extends PlayerInteractionManager
{
    private boolean field_73105_c;
    private boolean demoTimeExpired;
    private int field_73104_e;
    private int field_73102_f;

    public DemoWorldManager(World worldIn)
    {
        super(worldIn);
    }

    public void updateBlockRemoving()
    {
        super.updateBlockRemoving();
        ++this.field_73102_f;
        long i = this.theWorld.getTotalWorldTime();
        long j = i / 24000L + 1L;

        if (!this.field_73105_c && this.field_73102_f > 20)
        {
            this.field_73105_c = true;
            this.thisPlayerMP.playerNetServerHandler.sendPacket(new SPacketChangeGameState(5, 0.0F));
        }

        this.demoTimeExpired = i > 120500L;

        if (this.demoTimeExpired)
        {
            ++this.field_73104_e;
        }

        if (i % 24000L == 500L)
        {
            if (j <= 6L)
            {
                this.thisPlayerMP.addChatMessage(new TextComponentTranslation("demo.day." + j, new Object[0]));
            }
        }
        else if (j == 1L)
        {
            if (i == 100L)
            {
                this.thisPlayerMP.playerNetServerHandler.sendPacket(new SPacketChangeGameState(5, 101.0F));
            }
            else if (i == 175L)
            {
                this.thisPlayerMP.playerNetServerHandler.sendPacket(new SPacketChangeGameState(5, 102.0F));
            }
            else if (i == 250L)
            {
                this.thisPlayerMP.playerNetServerHandler.sendPacket(new SPacketChangeGameState(5, 103.0F));
            }
        }
        else if (j == 5L && i % 24000L == 22000L)
        {
            this.thisPlayerMP.addChatMessage(new TextComponentTranslation("demo.day.warning", new Object[0]));
        }
    }

    /**
     * Sends a message to the player reminding them that this is the demo version
     */
    private void sendDemoReminder()
    {
        if (this.field_73104_e > 100)
        {
            this.thisPlayerMP.addChatMessage(new TextComponentTranslation("demo.reminder", new Object[0]));
            this.field_73104_e = 0;
        }
    }

    /**
     * If not creative, it calls sendBlockBreakProgress until the block is broken first. tryHarvestBlock can also be the
     * result of this call.
     */
    public void onBlockClicked(BlockPos pos, EnumFacing side)
    {
        if (this.demoTimeExpired)
        {
            this.sendDemoReminder();
        }
        else
        {
            super.onBlockClicked(pos, side);
        }
    }

    public void blockRemoving(BlockPos pos)
    {
        if (!this.demoTimeExpired)
        {
            super.blockRemoving(pos);
        }
    }

    /**
     * Attempts to harvest a block
     */
    public boolean tryHarvestBlock(BlockPos pos)
    {
        return this.demoTimeExpired ? false : super.tryHarvestBlock(pos);
    }

    public EnumActionResult func_187250_a(EntityPlayer p_187250_1_, World p_187250_2_, ItemStack p_187250_3_, EnumHand p_187250_4_)
    {
        if (this.demoTimeExpired)
        {
            this.sendDemoReminder();
            return EnumActionResult.PASS;
        }
        else
        {
            return super.func_187250_a(p_187250_1_, p_187250_2_, p_187250_3_, p_187250_4_);
        }
    }

    public EnumActionResult func_187251_a(EntityPlayer p_187251_1_, World p_187251_2_, ItemStack p_187251_3_, EnumHand p_187251_4_, BlockPos p_187251_5_, EnumFacing p_187251_6_, float p_187251_7_, float p_187251_8_, float p_187251_9_)
    {
        if (this.demoTimeExpired)
        {
            this.sendDemoReminder();
            return EnumActionResult.PASS;
        }
        else
        {
            return super.func_187251_a(p_187251_1_, p_187251_2_, p_187251_3_, p_187251_4_, p_187251_5_, p_187251_6_, p_187251_7_, p_187251_8_, p_187251_9_);
        }
    }
}
