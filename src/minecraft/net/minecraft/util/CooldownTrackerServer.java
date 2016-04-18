package net.minecraft.util;

import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.item.Item;
import net.minecraft.network.play.server.SPacketCooldown;

public class CooldownTrackerServer extends CooldownTracker
{
    private final EntityPlayerMP player;

    public CooldownTrackerServer(EntityPlayerMP playerIn)
    {
        this.player = playerIn;
    }

    protected void notifyOnSet(Item itemIn, int p_185140_2_)
    {
        super.notifyOnSet(itemIn, p_185140_2_);
        this.player.playerNetServerHandler.sendPacket(new SPacketCooldown(itemIn, p_185140_2_));
    }

    protected void notifyOnRemove(Item itemIn)
    {
        super.notifyOnRemove(itemIn);
        this.player.playerNetServerHandler.sendPacket(new SPacketCooldown(itemIn, 0));
    }
}
