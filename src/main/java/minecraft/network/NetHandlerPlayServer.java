package net.minecraft.network;

import com.google.common.collect.Lists;
import com.google.common.primitives.Doubles;
import com.google.common.primitives.Floats;
import com.google.common.util.concurrent.Futures;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.util.concurrent.Future;
import io.netty.util.concurrent.GenericFutureListener;
import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import net.minecraft.block.BlockCommandBlock;
import net.minecraft.block.material.Material;
import net.minecraft.block.state.IBlockState;
import net.minecraft.crash.CrashReport;
import net.minecraft.crash.CrashReportCategory;
import net.minecraft.entity.Entity;
import net.minecraft.entity.IJumpingMount;
import net.minecraft.entity.item.EntityBoat;
import net.minecraft.entity.item.EntityItem;
import net.minecraft.entity.item.EntityMinecartCommandBlock;
import net.minecraft.entity.item.EntityXPOrb;
import net.minecraft.entity.passive.EntityHorse;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.entity.player.InventoryPlayer;
import net.minecraft.entity.projectile.EntityArrow;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.init.MobEffects;
import net.minecraft.inventory.Container;
import net.minecraft.inventory.ContainerBeacon;
import net.minecraft.inventory.ContainerMerchant;
import net.minecraft.inventory.ContainerRepair;
import net.minecraft.inventory.EntityEquipmentSlot;
import net.minecraft.inventory.IInventory;
import net.minecraft.inventory.Slot;
import net.minecraft.item.ItemElytra;
import net.minecraft.item.ItemStack;
import net.minecraft.item.ItemWritableBook;
import net.minecraft.item.ItemWrittenBook;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.nbt.NBTTagString;
import net.minecraft.network.play.INetHandlerPlayServer;
import net.minecraft.network.play.client.CPacketAnimation;
import net.minecraft.network.play.client.CPacketChatMessage;
import net.minecraft.network.play.client.CPacketClickWindow;
import net.minecraft.network.play.client.CPacketClientSettings;
import net.minecraft.network.play.client.CPacketClientStatus;
import net.minecraft.network.play.client.CPacketCloseWindow;
import net.minecraft.network.play.client.CPacketConfirmTeleport;
import net.minecraft.network.play.client.CPacketConfirmTransaction;
import net.minecraft.network.play.client.CPacketCreativeInventoryAction;
import net.minecraft.network.play.client.CPacketCustomPayload;
import net.minecraft.network.play.client.CPacketEnchantItem;
import net.minecraft.network.play.client.CPacketEntityAction;
import net.minecraft.network.play.client.CPacketHeldItemChange;
import net.minecraft.network.play.client.CPacketInput;
import net.minecraft.network.play.client.CPacketKeepAlive;
import net.minecraft.network.play.client.CPacketPlayer;
import net.minecraft.network.play.client.CPacketPlayerAbilities;
import net.minecraft.network.play.client.CPacketPlayerBlockPlacement;
import net.minecraft.network.play.client.CPacketPlayerDigging;
import net.minecraft.network.play.client.CPacketPlayerTryUseItem;
import net.minecraft.network.play.client.CPacketResourcePackStatus;
import net.minecraft.network.play.client.CPacketSpectate;
import net.minecraft.network.play.client.CPacketSteerBoat;
import net.minecraft.network.play.client.CPacketTabComplete;
import net.minecraft.network.play.client.CPacketUpdateSign;
import net.minecraft.network.play.client.CPacketUseEntity;
import net.minecraft.network.play.client.CPacketVehicleMove;
import net.minecraft.network.play.server.SPacketBlockChange;
import net.minecraft.network.play.server.SPacketChat;
import net.minecraft.network.play.server.SPacketConfirmTransaction;
import net.minecraft.network.play.server.SPacketDisconnect;
import net.minecraft.network.play.server.SPacketHeldItemChange;
import net.minecraft.network.play.server.SPacketKeepAlive;
import net.minecraft.network.play.server.SPacketMoveVehicle;
import net.minecraft.network.play.server.SPacketPlayerPosLook;
import net.minecraft.network.play.server.SPacketRespawn;
import net.minecraft.network.play.server.SPacketSetSlot;
import net.minecraft.network.play.server.SPacketTabComplete;
import net.minecraft.server.MinecraftServer;
import net.minecraft.stats.AchievementList;
import net.minecraft.tileentity.CommandBlockBaseLogic;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityCommandBlock;
import net.minecraft.tileentity.TileEntitySign;
import net.minecraft.tileentity.TileEntityStructure;
import net.minecraft.util.ChatAllowedCharacters;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.EnumHand;
import net.minecraft.util.ITickable;
import net.minecraft.util.IntHashMap;
import net.minecraft.util.Mirror;
import net.minecraft.util.ReportedException;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.util.text.ITextComponent;
import net.minecraft.util.text.TextComponentString;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.util.text.TextFormatting;
import net.minecraft.world.WorldServer;
import net.minecraft.world.WorldSettings;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class NetHandlerPlayServer implements INetHandlerPlayServer, ITickable
{
    private static final Logger logger = LogManager.getLogger();
    public final NetworkManager netManager;
    private final MinecraftServer serverController;
    public EntityPlayerMP playerEntity;
    private int networkTickCount;
    private int field_147378_h;
    private long lastPingTime;
    private long lastSentPingPacket;

    /**
     * Incremented by 20 each time a user sends a chat message, decreased by one every tick. Non-ops kicked when over
     * 200
     */
    private int chatSpamThresholdCount;
    private int itemDropThreshold;
    private final IntHashMap<Short> field_147372_n = new IntHashMap();
    private double field_184349_l;
    private double field_184350_m;
    private double field_184351_n;
    private double field_184352_o;
    private double field_184353_p;
    private double field_184354_q;
    private Entity field_184355_r;
    private double field_184356_s;
    private double field_184357_t;
    private double field_184358_u;
    private double field_184359_v;
    private double field_184360_w;
    private double field_184361_x;
    private Vec3d field_184362_y;
    private int field_184363_z;
    private int field_184343_A;
    private boolean field_184344_B;

    /**
     * Used to keep track of how the player is floating while gamerules should prevent that. Surpassing 80 ticks means
     * kick
     */
    private int floatingTickCount;
    private boolean field_184345_D;
    private int field_184346_E;
    private int field_184347_F;
    private int field_184348_G;

    public NetHandlerPlayServer(MinecraftServer server, NetworkManager networkManagerIn, EntityPlayerMP playerIn)
    {
        this.serverController = server;
        this.netManager = networkManagerIn;
        networkManagerIn.setNetHandler(this);
        this.playerEntity = playerIn;
        playerIn.playerNetServerHandler = this;
    }

    /**
     * Like the old updateEntity(), except more generic.
     */
    public void update()
    {
        this.func_184342_d();
        this.playerEntity.onUpdateEntity();
        this.playerEntity.setPositionAndRotation(this.field_184349_l, this.field_184350_m, this.field_184351_n, this.playerEntity.rotationYaw, this.playerEntity.rotationPitch);
        ++this.networkTickCount;
        this.field_184348_G = this.field_184347_F;

        if (this.field_184344_B)
        {
            if (++this.floatingTickCount > 80)
            {
                logger.warn(this.playerEntity.getName() + " was kicked for floating too long!");
                this.kickPlayerFromServer("Flying is not enabled on this server");
                return;
            }
        }
        else
        {
            this.field_184344_B = false;
            this.floatingTickCount = 0;
        }

        this.field_184355_r = this.playerEntity.getLowestRidingEntity();

        if (this.field_184355_r != this.playerEntity && this.field_184355_r.getControllingPassenger() == this.playerEntity)
        {
            this.field_184356_s = this.field_184355_r.posX;
            this.field_184357_t = this.field_184355_r.posY;
            this.field_184358_u = this.field_184355_r.posZ;
            this.field_184359_v = this.field_184355_r.posX;
            this.field_184360_w = this.field_184355_r.posY;
            this.field_184361_x = this.field_184355_r.posZ;

            if (this.field_184345_D && this.playerEntity.getLowestRidingEntity().getControllingPassenger() == this.playerEntity)
            {
                if (++this.field_184346_E > 80)
                {
                    logger.warn(this.playerEntity.getName() + " was kicked for floating a vehicle too long!");
                    this.kickPlayerFromServer("Flying is not enabled on this server");
                    return;
                }
            }
            else
            {
                this.field_184345_D = false;
                this.field_184346_E = 0;
            }
        }
        else
        {
            this.field_184355_r = null;
            this.field_184345_D = false;
            this.field_184346_E = 0;
        }

        this.serverController.theProfiler.startSection("keepAlive");

        if ((long)this.networkTickCount - this.lastSentPingPacket > 40L)
        {
            this.lastSentPingPacket = (long)this.networkTickCount;
            this.lastPingTime = this.currentTimeMillis();
            this.field_147378_h = (int)this.lastPingTime;
            this.sendPacket(new SPacketKeepAlive(this.field_147378_h));
        }

        this.serverController.theProfiler.endSection();

        if (this.chatSpamThresholdCount > 0)
        {
            --this.chatSpamThresholdCount;
        }

        if (this.itemDropThreshold > 0)
        {
            --this.itemDropThreshold;
        }

        if (this.playerEntity.getLastActiveTime() > 0L && this.serverController.getMaxPlayerIdleMinutes() > 0 && MinecraftServer.getCurrentTimeMillis() - this.playerEntity.getLastActiveTime() > (long)(this.serverController.getMaxPlayerIdleMinutes() * 1000 * 60))
        {
            this.kickPlayerFromServer("You have been idle for too long!");
        }
    }

    private void func_184342_d()
    {
        this.field_184349_l = this.playerEntity.posX;
        this.field_184350_m = this.playerEntity.posY;
        this.field_184351_n = this.playerEntity.posZ;
        this.field_184352_o = this.playerEntity.posX;
        this.field_184353_p = this.playerEntity.posY;
        this.field_184354_q = this.playerEntity.posZ;
    }

    public NetworkManager getNetworkManager()
    {
        return this.netManager;
    }

    /**
     * Kick a player from the server with a reason
     */
    public void kickPlayerFromServer(String reason)
    {
        final TextComponentString textcomponentstring = new TextComponentString(reason);
        this.netManager.sendPacket(new SPacketDisconnect(textcomponentstring), new GenericFutureListener < Future <? super Void >> ()
        {
            public void operationComplete(Future <? super Void > p_operationComplete_1_) throws Exception
            {
                NetHandlerPlayServer.this.netManager.closeChannel(textcomponentstring);
            }
        }, new GenericFutureListener[0]);
        this.netManager.disableAutoRead();
        Futures.getUnchecked(this.serverController.addScheduledTask(new Runnable()
        {
            public void run()
            {
                NetHandlerPlayServer.this.netManager.checkDisconnected();
            }
        }));
    }

    /**
     * Processes player movement input. Includes walking, strafing, jumping, sneaking; excludes riding and toggling
     * flying/sprinting
     */
    public void processInput(CPacketInput packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.setEntityActionState(packetIn.getStrafeSpeed(), packetIn.getForwardSpeed(), packetIn.isJumping(), packetIn.isSneaking());
    }

    private static boolean isMovePlayerPacketInvalid(CPacketPlayer packetIn)
    {
        return Doubles.isFinite(packetIn.func_186997_a(0.0D)) && Doubles.isFinite(packetIn.func_186996_b(0.0D)) && Doubles.isFinite(packetIn.func_187000_c(0.0D)) && Floats.isFinite(packetIn.func_186998_b(0.0F)) && Floats.isFinite(packetIn.func_186999_a(0.0F)) ? false : Math.abs(packetIn.func_186997_a(0.0D)) <= 3.0E7D && Math.abs(packetIn.func_186997_a(0.0D)) <= 3.0E7D;
    }

    private static boolean isMoveVehiclePacketInvalid(CPacketVehicleMove packetIn)
    {
        return !Doubles.isFinite(packetIn.getX()) || !Doubles.isFinite(packetIn.getY()) || !Doubles.isFinite(packetIn.getZ()) || !Floats.isFinite(packetIn.getPitch()) || !Floats.isFinite(packetIn.getYaw());
    }

    public void processVehicleMove(CPacketVehicleMove packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (isMoveVehiclePacketInvalid(packetIn))
        {
            this.kickPlayerFromServer("Invalid move vehicle packet received");
        }
        else
        {
            Entity entity = this.playerEntity.getLowestRidingEntity();

            if (entity != this.playerEntity && entity.getControllingPassenger() == this.playerEntity && entity == this.field_184355_r)
            {
                WorldServer worldserver = this.playerEntity.getServerForPlayer();
                double d0 = entity.posX;
                double d1 = entity.posY;
                double d2 = entity.posZ;
                double d3 = packetIn.getX();
                double d4 = packetIn.getY();
                double d5 = packetIn.getZ();
                float f = packetIn.getYaw();
                float f1 = packetIn.getPitch();
                double d6 = d3 - this.field_184356_s;
                double d7 = d4 - this.field_184357_t;
                double d8 = d5 - this.field_184358_u;
                double d9 = entity.motionX * entity.motionX + entity.motionY * entity.motionY + entity.motionZ * entity.motionZ;
                double d10 = d6 * d6 + d7 * d7 + d8 * d8;

                if (d10 - d9 > 100.0D && (!this.serverController.isSinglePlayer() || !this.serverController.getServerOwner().equals(entity.getName())))
                {
                    logger.warn(entity.getName() + " (vehicle of " + this.playerEntity.getName() + ") moved too quickly! " + d6 + "," + d7 + "," + d8);
                    this.netManager.sendPacket(new SPacketMoveVehicle(entity));
                    return;
                }

                boolean flag = worldserver.func_184144_a(entity, entity.getEntityBoundingBox().func_186664_h(0.0625D)).isEmpty();
                d6 = d3 - this.field_184359_v;
                d7 = d4 - this.field_184360_w - 1.0E-6D;
                d8 = d5 - this.field_184361_x;
                entity.moveEntity(d6, d7, d8);
                double d11 = d7;
                d6 = d3 - entity.posX;
                d7 = d4 - entity.posY;

                if (d7 > -0.5D || d7 < 0.5D)
                {
                    d7 = 0.0D;
                }

                d8 = d5 - entity.posZ;
                d10 = d6 * d6 + d7 * d7 + d8 * d8;
                boolean flag1 = false;

                if (d10 > 0.0625D)
                {
                    flag1 = true;
                    logger.warn(entity.getName() + " moved wrongly!");
                }

                entity.setPositionAndRotation(d3, d4, d5, f, f1);
                boolean flag2 = worldserver.func_184144_a(entity, entity.getEntityBoundingBox().func_186664_h(0.0625D)).isEmpty();

                if (flag && (flag1 || !flag2))
                {
                    entity.setPositionAndRotation(d0, d1, d2, f, f1);
                    this.netManager.sendPacket(new SPacketMoveVehicle(entity));
                    return;
                }

                this.serverController.getPlayerList().serverUpdateMountedMovingPlayer(this.playerEntity);
                this.playerEntity.addMovementStat(this.playerEntity.posX - d0, this.playerEntity.posY - d1, this.playerEntity.posZ - d2);
                this.field_184345_D = d11 >= -0.03125D && !this.serverController.isFlightAllowed() && !worldserver.checkBlockCollision(entity.getEntityBoundingBox().func_186662_g(0.0625D).addCoord(0.0D, -0.55D, 0.0D));
                this.field_184359_v = entity.posX;
                this.field_184360_w = entity.posY;
                this.field_184361_x = entity.posZ;
            }
        }
    }

    public void func_184339_a(CPacketConfirmTeleport packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (packetIn.getTeleportId() == this.field_184363_z)
        {
            this.playerEntity.setPositionAndRotation(this.field_184362_y.xCoord, this.field_184362_y.yCoord, this.field_184362_y.zCoord, this.playerEntity.rotationYaw, this.playerEntity.rotationPitch);

            if (this.playerEntity.func_184850_K())
            {
                this.field_184352_o = this.field_184362_y.xCoord;
                this.field_184353_p = this.field_184362_y.yCoord;
                this.field_184354_q = this.field_184362_y.zCoord;
                this.playerEntity.func_184846_L();
            }

            this.field_184362_y = null;
        }
    }

    /**
     * Processes clients perspective on player positioning and/or orientation
     */
    public void processPlayer(CPacketPlayer packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (isMovePlayerPacketInvalid(packetIn))
        {
            this.kickPlayerFromServer("Invalid move player packet received");
        }
        else
        {
            WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);

            if (!this.playerEntity.playerConqueredTheEnd)
            {
                if (this.networkTickCount == 0)
                {
                    this.func_184342_d();
                }

                if (this.field_184362_y != null)
                {
                    if (this.networkTickCount - this.field_184343_A > 20)
                    {
                        this.field_184343_A = this.networkTickCount;
                        this.setPlayerLocation(this.field_184362_y.xCoord, this.field_184362_y.yCoord, this.field_184362_y.zCoord, this.playerEntity.rotationYaw, this.playerEntity.rotationPitch);
                    }
                }
                else
                {
                    this.field_184343_A = this.networkTickCount;

                    if (this.playerEntity.isRiding())
                    {
                        this.playerEntity.setPositionAndRotation(this.playerEntity.posX, this.playerEntity.posY, this.playerEntity.posZ, packetIn.func_186999_a(this.playerEntity.rotationYaw), packetIn.func_186998_b(this.playerEntity.rotationPitch));
                        this.serverController.getPlayerList().serverUpdateMountedMovingPlayer(this.playerEntity);
                    }
                    else
                    {
                        double d0 = this.playerEntity.posX;
                        double d1 = this.playerEntity.posY;
                        double d2 = this.playerEntity.posZ;
                        double d3 = this.playerEntity.posY;
                        double d4 = packetIn.func_186997_a(this.playerEntity.posX);
                        double d5 = packetIn.func_186996_b(this.playerEntity.posY);
                        double d6 = packetIn.func_187000_c(this.playerEntity.posZ);
                        float f = packetIn.func_186999_a(this.playerEntity.rotationYaw);
                        float f1 = packetIn.func_186998_b(this.playerEntity.rotationPitch);
                        double d7 = d4 - this.field_184349_l;
                        double d8 = d5 - this.field_184350_m;
                        double d9 = d6 - this.field_184351_n;
                        double d10 = this.playerEntity.motionX * this.playerEntity.motionX + this.playerEntity.motionY * this.playerEntity.motionY + this.playerEntity.motionZ * this.playerEntity.motionZ;
                        double d11 = d7 * d7 + d8 * d8 + d9 * d9;
                        ++this.field_184347_F;
                        int i = this.field_184347_F - this.field_184348_G;

                        if (i > 5)
                        {
                            logger.debug(this.playerEntity.getName() + " is sending move packets too frequently (" + i + " packets since last tick)");
                            i = 1;
                        }

                        if (!this.playerEntity.func_184850_K() && (!this.playerEntity.getServerForPlayer().getGameRules().getBoolean("disableElytraMovementCheck") || !this.playerEntity.func_184613_cA()))
                        {
                            float f2 = this.playerEntity.func_184613_cA() ? 300.0F : 100.0F;

                            if (d11 - d10 > (double)(f2 * (float)i) && (!this.serverController.isSinglePlayer() || !this.serverController.getServerOwner().equals(this.playerEntity.getName())))
                            {
                                logger.warn(this.playerEntity.getName() + " moved too quickly! " + d7 + "," + d8 + "," + d9);
                                this.setPlayerLocation(this.playerEntity.posX, this.playerEntity.posY, this.playerEntity.posZ, this.playerEntity.rotationYaw, this.playerEntity.rotationPitch);
                                return;
                            }
                        }

                        boolean flag2 = worldserver.func_184144_a(this.playerEntity, this.playerEntity.getEntityBoundingBox().func_186664_h(0.0625D)).isEmpty();
                        d7 = d4 - this.field_184352_o;
                        d8 = d5 - this.field_184353_p;
                        d9 = d6 - this.field_184354_q;

                        if (this.playerEntity.onGround && !packetIn.isOnGround() && d8 > 0.0D)
                        {
                            this.playerEntity.jump();
                        }

                        this.playerEntity.moveEntity(d7, d8, d9);
                        this.playerEntity.onGround = packetIn.isOnGround();
                        double d12 = d8;
                        d7 = d4 - this.playerEntity.posX;
                        d8 = d5 - this.playerEntity.posY;

                        if (d8 > -0.5D || d8 < 0.5D)
                        {
                            d8 = 0.0D;
                        }

                        d9 = d6 - this.playerEntity.posZ;
                        d11 = d7 * d7 + d8 * d8 + d9 * d9;
                        boolean flag = false;

                        if (!this.playerEntity.func_184850_K() && d11 > 0.0625D && !this.playerEntity.isPlayerSleeping() && !this.playerEntity.theItemInWorldManager.isCreative() && this.playerEntity.theItemInWorldManager.getGameType() != WorldSettings.GameType.SPECTATOR)
                        {
                            flag = true;
                            logger.warn(this.playerEntity.getName() + " moved wrongly!");
                        }

                        this.playerEntity.setPositionAndRotation(d4, d5, d6, f, f1);
                        this.playerEntity.addMovementStat(this.playerEntity.posX - d0, this.playerEntity.posY - d1, this.playerEntity.posZ - d2);

                        if (!this.playerEntity.noClip && !this.playerEntity.isPlayerSleeping())
                        {
                            boolean flag1 = worldserver.func_184144_a(this.playerEntity, this.playerEntity.getEntityBoundingBox().func_186664_h(0.0625D)).isEmpty();

                            if (flag2 && (flag || !flag1))
                            {
                                this.setPlayerLocation(d0, d1, d2, f, f1);
                                return;
                            }
                        }

                        this.field_184344_B = d12 >= -0.03125D;
                        this.field_184344_B &= !this.serverController.isFlightAllowed() && !this.playerEntity.capabilities.allowFlying;
                        this.field_184344_B &= !this.playerEntity.isPotionActive(MobEffects.levitation) && !this.playerEntity.func_184613_cA() && !worldserver.checkBlockCollision(this.playerEntity.getEntityBoundingBox().func_186662_g(0.0625D).addCoord(0.0D, -0.55D, 0.0D));
                        this.playerEntity.onGround = packetIn.isOnGround();
                        this.serverController.getPlayerList().serverUpdateMountedMovingPlayer(this.playerEntity);
                        this.playerEntity.handleFalling(this.playerEntity.posY - d3, packetIn.isOnGround());
                        this.field_184352_o = this.playerEntity.posX;
                        this.field_184353_p = this.playerEntity.posY;
                        this.field_184354_q = this.playerEntity.posZ;
                    }
                }
            }
        }
    }

    public void setPlayerLocation(double x, double y, double z, float yaw, float pitch)
    {
        this.setPlayerLocation(x, y, z, yaw, pitch, Collections.<SPacketPlayerPosLook.EnumFlags>emptySet());
    }

    public void setPlayerLocation(double x, double y, double z, float yaw, float pitch, Set<SPacketPlayerPosLook.EnumFlags> relativeSet)
    {
        double d0 = relativeSet.contains(SPacketPlayerPosLook.EnumFlags.X) ? this.playerEntity.posX : 0.0D;
        double d1 = relativeSet.contains(SPacketPlayerPosLook.EnumFlags.Y) ? this.playerEntity.posY : 0.0D;
        double d2 = relativeSet.contains(SPacketPlayerPosLook.EnumFlags.Z) ? this.playerEntity.posZ : 0.0D;
        this.field_184362_y = new Vec3d(x + d0, y + d1, z + d2);
        float f = yaw;
        float f1 = pitch;

        if (relativeSet.contains(SPacketPlayerPosLook.EnumFlags.Y_ROT))
        {
            f = yaw + this.playerEntity.rotationYaw;
        }

        if (relativeSet.contains(SPacketPlayerPosLook.EnumFlags.X_ROT))
        {
            f1 = pitch + this.playerEntity.rotationPitch;
        }

        if (++this.field_184363_z == Integer.MAX_VALUE)
        {
            this.field_184363_z = 0;
        }

        this.field_184343_A = this.networkTickCount;
        this.playerEntity.setPositionAndRotation(this.field_184362_y.xCoord, this.field_184362_y.yCoord, this.field_184362_y.zCoord, f, f1);
        this.playerEntity.playerNetServerHandler.sendPacket(new SPacketPlayerPosLook(x, y, z, yaw, pitch, relativeSet, this.field_184363_z));
    }

    /**
     * Processes the player initiating/stopping digging on a particular spot, as well as a player dropping items?. (0:
     * initiated, 1: reinitiated, 2? , 3-4 drop item (respectively without or with player control), 5: stopped; x,y,z,
     * side clicked on;)
     */
    public void processPlayerDigging(CPacketPlayerDigging packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);
        BlockPos blockpos = packetIn.getPosition();
        this.playerEntity.markPlayerActive();

        switch (packetIn.getStatus())
        {
            case SWAP_HELD_ITEMS:
                if (!this.playerEntity.isSpectator())
                {
                    ItemStack itemstack1 = this.playerEntity.getHeldItem(EnumHand.OFF_HAND);
                    this.playerEntity.setHeldItem(EnumHand.OFF_HAND, this.playerEntity.getHeldItem(EnumHand.MAIN_HAND));
                    this.playerEntity.setHeldItem(EnumHand.MAIN_HAND, itemstack1);
                }

                return;

            case DROP_ITEM:
                if (!this.playerEntity.isSpectator())
                {
                    this.playerEntity.dropOneItem(false);
                }

                return;

            case DROP_ALL_ITEMS:
                if (!this.playerEntity.isSpectator())
                {
                    this.playerEntity.dropOneItem(true);
                }

                return;

            case RELEASE_USE_ITEM:
                this.playerEntity.func_184597_cx();
                ItemStack itemstack = this.playerEntity.getHeldItemMainhand();

                if (itemstack != null && itemstack.stackSize == 0)
                {
                    this.playerEntity.setHeldItem(EnumHand.MAIN_HAND, (ItemStack)null);
                }

                return;

            case START_DESTROY_BLOCK:
            case ABORT_DESTROY_BLOCK:
            case STOP_DESTROY_BLOCK:
                double d0 = this.playerEntity.posX - ((double)blockpos.getX() + 0.5D);
                double d1 = this.playerEntity.posY - ((double)blockpos.getY() + 0.5D) + 1.5D;
                double d2 = this.playerEntity.posZ - ((double)blockpos.getZ() + 0.5D);
                double d3 = d0 * d0 + d1 * d1 + d2 * d2;

                if (d3 > 36.0D)
                {
                    return;
                }
                else if (blockpos.getY() >= this.serverController.getBuildLimit())
                {
                    return;
                }
                else
                {
                    if (packetIn.getStatus() == CPacketPlayerDigging.Action.START_DESTROY_BLOCK)
                    {
                        if (!this.serverController.isBlockProtected(worldserver, blockpos, this.playerEntity) && worldserver.getWorldBorder().contains(blockpos))
                        {
                            this.playerEntity.theItemInWorldManager.onBlockClicked(blockpos, packetIn.getFacing());
                        }
                        else
                        {
                            this.playerEntity.playerNetServerHandler.sendPacket(new SPacketBlockChange(worldserver, blockpos));
                        }
                    }
                    else
                    {
                        if (packetIn.getStatus() == CPacketPlayerDigging.Action.STOP_DESTROY_BLOCK)
                        {
                            this.playerEntity.theItemInWorldManager.blockRemoving(blockpos);
                        }
                        else if (packetIn.getStatus() == CPacketPlayerDigging.Action.ABORT_DESTROY_BLOCK)
                        {
                            this.playerEntity.theItemInWorldManager.cancelDestroyingBlock();
                        }

                        if (worldserver.getBlockState(blockpos).getMaterial() != Material.air)
                        {
                            this.playerEntity.playerNetServerHandler.sendPacket(new SPacketBlockChange(worldserver, blockpos));
                        }
                    }

                    return;
                }

            default:
                throw new IllegalArgumentException("Invalid player action");
        }
    }

    public void func_184337_a(CPacketPlayerTryUseItem packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);
        EnumHand enumhand = packetIn.getHand();
        ItemStack itemstack = this.playerEntity.getHeldItem(enumhand);
        BlockPos blockpos = packetIn.func_187023_a();
        EnumFacing enumfacing = packetIn.func_187024_b();
        this.playerEntity.markPlayerActive();

        if (blockpos.getY() < this.serverController.getBuildLimit() - 1 || enumfacing != EnumFacing.UP && blockpos.getY() < this.serverController.getBuildLimit())
        {
            if (this.field_184362_y == null && this.playerEntity.getDistanceSq((double)blockpos.getX() + 0.5D, (double)blockpos.getY() + 0.5D, (double)blockpos.getZ() + 0.5D) < 64.0D && !this.serverController.isBlockProtected(worldserver, blockpos, this.playerEntity) && worldserver.getWorldBorder().contains(blockpos))
            {
                this.playerEntity.theItemInWorldManager.func_187251_a(this.playerEntity, worldserver, itemstack, enumhand, blockpos, enumfacing, packetIn.func_187026_d(), packetIn.func_187025_e(), packetIn.func_187020_f());
            }
        }
        else
        {
            TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("build.tooHigh", new Object[] {Integer.valueOf(this.serverController.getBuildLimit())});
            textcomponenttranslation.getChatStyle().setColor(TextFormatting.RED);
            this.playerEntity.playerNetServerHandler.sendPacket(new SPacketChat(textcomponenttranslation));
        }

        this.playerEntity.playerNetServerHandler.sendPacket(new SPacketBlockChange(worldserver, blockpos));
        this.playerEntity.playerNetServerHandler.sendPacket(new SPacketBlockChange(worldserver, blockpos.offset(enumfacing)));
        itemstack = this.playerEntity.getHeldItem(enumhand);

        if (itemstack != null && itemstack.stackSize == 0)
        {
            this.playerEntity.setHeldItem(enumhand, (ItemStack)null);
            itemstack = null;
        }
    }

    /**
     * Processes block placement and block activation (anvil, furnace, etc.)
     */
    public void processPlayerBlockPlacement(CPacketPlayerBlockPlacement packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);
        EnumHand enumhand = packetIn.getHand();
        ItemStack itemstack = this.playerEntity.getHeldItem(enumhand);
        this.playerEntity.markPlayerActive();

        if (itemstack != null)
        {
            this.playerEntity.theItemInWorldManager.func_187250_a(this.playerEntity, worldserver, itemstack, enumhand);
            itemstack = this.playerEntity.getHeldItem(enumhand);

            if (itemstack != null && itemstack.stackSize == 0)
            {
                this.playerEntity.setHeldItem(enumhand, (ItemStack)null);
                itemstack = null;
            }
        }
    }

    public void handleSpectate(CPacketSpectate packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (this.playerEntity.isSpectator())
        {
            Entity entity = null;

            for (WorldServer worldserver : this.serverController.worldServers)
            {
                if (worldserver != null)
                {
                    entity = packetIn.getEntity(worldserver);

                    if (entity != null)
                    {
                        break;
                    }
                }
            }

            if (entity != null)
            {
                this.playerEntity.setSpectatingEntity(this.playerEntity);
                this.playerEntity.dismountRidingEntity();

                if (entity.worldObj != this.playerEntity.worldObj)
                {
                    WorldServer worldserver1 = this.playerEntity.getServerForPlayer();
                    WorldServer worldserver2 = (WorldServer)entity.worldObj;
                    this.playerEntity.dimension = entity.dimension;
                    this.sendPacket(new SPacketRespawn(this.playerEntity.dimension, worldserver1.getDifficulty(), worldserver1.getWorldInfo().getTerrainType(), this.playerEntity.theItemInWorldManager.getGameType()));
                    this.serverController.getPlayerList().func_187243_f(this.playerEntity);
                    worldserver1.removePlayerEntityDangerously(this.playerEntity);
                    this.playerEntity.isDead = false;
                    this.playerEntity.setLocationAndAngles(entity.posX, entity.posY, entity.posZ, entity.rotationYaw, entity.rotationPitch);

                    if (this.playerEntity.isEntityAlive())
                    {
                        worldserver1.updateEntityWithOptionalForce(this.playerEntity, false);
                        worldserver2.spawnEntityInWorld(this.playerEntity);
                        worldserver2.updateEntityWithOptionalForce(this.playerEntity, false);
                    }

                    this.playerEntity.setWorld(worldserver2);
                    this.serverController.getPlayerList().preparePlayer(this.playerEntity, worldserver1);
                    this.playerEntity.setPositionAndUpdate(entity.posX, entity.posY, entity.posZ);
                    this.playerEntity.theItemInWorldManager.setWorld(worldserver2);
                    this.serverController.getPlayerList().updateTimeAndWeatherForPlayer(this.playerEntity, worldserver2);
                    this.serverController.getPlayerList().syncPlayerInventory(this.playerEntity);
                }
                else
                {
                    this.playerEntity.setPositionAndUpdate(entity.posX, entity.posY, entity.posZ);
                }
            }
        }
    }

    public void handleResourcePackStatus(CPacketResourcePackStatus packetIn)
    {
    }

    public void func_184340_a(CPacketSteerBoat packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        Entity entity = this.playerEntity.getRidingEntity();

        if (entity instanceof EntityBoat)
        {
            ((EntityBoat)entity).func_184445_a(packetIn.func_187012_a(), packetIn.func_187014_b());
        }
    }

    /**
     * Invoked when disconnecting, the parameter is a ChatComponent describing the reason for termination
     */
    public void onDisconnect(ITextComponent reason)
    {
        logger.info(this.playerEntity.getName() + " lost connection: " + reason);
        this.serverController.refreshStatusNextTick();
        TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("multiplayer.player.left", new Object[] {this.playerEntity.getDisplayName()});
        textcomponenttranslation.getChatStyle().setColor(TextFormatting.YELLOW);
        this.serverController.getPlayerList().sendChatMsg(textcomponenttranslation);
        this.playerEntity.mountEntityAndWakeUp();
        this.serverController.getPlayerList().playerLoggedOut(this.playerEntity);

        if (this.serverController.isSinglePlayer() && this.playerEntity.getName().equals(this.serverController.getServerOwner()))
        {
            logger.info("Stopping singleplayer server as player logged out");
            this.serverController.initiateShutdown();
        }
    }

    public void sendPacket(final Packet<?> packetIn)
    {
        if (packetIn instanceof SPacketChat)
        {
            SPacketChat spacketchat = (SPacketChat)packetIn;
            EntityPlayer.EnumChatVisibility entityplayer$enumchatvisibility = this.playerEntity.getChatVisibility();

            if (entityplayer$enumchatvisibility == EntityPlayer.EnumChatVisibility.HIDDEN)
            {
                return;
            }

            if (entityplayer$enumchatvisibility == EntityPlayer.EnumChatVisibility.SYSTEM && !spacketchat.isChat())
            {
                return;
            }
        }

        try
        {
            this.netManager.sendPacket(packetIn);
        }
        catch (Throwable throwable)
        {
            CrashReport crashreport = CrashReport.makeCrashReport(throwable, "Sending packet");
            CrashReportCategory crashreportcategory = crashreport.makeCategory("Packet being sent");
            crashreportcategory.addCrashSectionCallable("Packet class", new Callable<String>()
            {
                public String call() throws Exception
                {
                    return packetIn.getClass().getCanonicalName();
                }
            });
            throw new ReportedException(crashreport);
        }
    }

    /**
     * Updates which quickbar slot is selected
     */
    public void processHeldItemChange(CPacketHeldItemChange packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (packetIn.getSlotId() >= 0 && packetIn.getSlotId() < InventoryPlayer.getHotbarSize())
        {
            this.playerEntity.inventory.currentItem = packetIn.getSlotId();
            this.playerEntity.markPlayerActive();
        }
        else
        {
            logger.warn(this.playerEntity.getName() + " tried to set an invalid carried item");
        }
    }

    /**
     * Process chat messages (broadcast back to clients) and commands (executes)
     */
    public void processChatMessage(CPacketChatMessage packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (this.playerEntity.getChatVisibility() == EntityPlayer.EnumChatVisibility.HIDDEN)
        {
            TextComponentTranslation textcomponenttranslation = new TextComponentTranslation("chat.cannotSend", new Object[0]);
            textcomponenttranslation.getChatStyle().setColor(TextFormatting.RED);
            this.sendPacket(new SPacketChat(textcomponenttranslation));
        }
        else
        {
            this.playerEntity.markPlayerActive();
            String s = packetIn.getMessage();
            s = StringUtils.normalizeSpace(s);

            for (int i = 0; i < s.length(); ++i)
            {
                if (!ChatAllowedCharacters.isAllowedCharacter(s.charAt(i)))
                {
                    this.kickPlayerFromServer("Illegal characters in chat");
                    return;
                }
            }

            if (s.startsWith("/"))
            {
                this.handleSlashCommand(s);
            }
            else
            {
                ITextComponent itextcomponent = new TextComponentTranslation("chat.type.text", new Object[] {this.playerEntity.getDisplayName(), s});
                this.serverController.getPlayerList().sendChatMsgImpl(itextcomponent, false);
            }

            this.chatSpamThresholdCount += 20;

            if (this.chatSpamThresholdCount > 200 && !this.serverController.getPlayerList().canSendCommands(this.playerEntity.getGameProfile()))
            {
                this.kickPlayerFromServer("disconnect.spam");
            }
        }
    }

    /**
     * Handle commands that start with a /
     */
    private void handleSlashCommand(String command)
    {
        this.serverController.getCommandManager().executeCommand(this.playerEntity, command);
    }

    public void handleAnimation(CPacketAnimation packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();
        this.playerEntity.swingArm(packetIn.getHand());
    }

    /**
     * Processes a range of action-types: sneaking, sprinting, waking from sleep, opening the inventory or setting jump
     * height of the horse the player is riding
     */
    public void processEntityAction(CPacketEntityAction packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();

        switch (packetIn.getAction())
        {
            case START_SNEAKING:
                this.playerEntity.setSneaking(true);
                break;

            case STOP_SNEAKING:
                this.playerEntity.setSneaking(false);
                break;

            case START_SPRINTING:
                this.playerEntity.setSprinting(true);
                break;

            case STOP_SPRINTING:
                this.playerEntity.setSprinting(false);
                break;

            case STOP_SLEEPING:
                this.playerEntity.wakeUpPlayer(false, true, true);
                this.field_184362_y = new Vec3d(this.playerEntity.posX, this.playerEntity.posY, this.playerEntity.posZ);
                break;

            case START_RIDING_JUMP:
                if (this.playerEntity.getRidingEntity() instanceof IJumpingMount)
                {
                    IJumpingMount ijumpingmount1 = (IJumpingMount)this.playerEntity.getRidingEntity();
                    int i = packetIn.getAuxData();

                    if (ijumpingmount1.func_184776_b() && i > 0)
                    {
                        ijumpingmount1.func_184775_b(i);
                    }
                }

                break;

            case STOP_RIDING_JUMP:
                if (this.playerEntity.getRidingEntity() instanceof IJumpingMount)
                {
                    IJumpingMount ijumpingmount = (IJumpingMount)this.playerEntity.getRidingEntity();
                    ijumpingmount.func_184777_r_();
                }

                break;

            case OPEN_INVENTORY:
                if (this.playerEntity.getRidingEntity() instanceof EntityHorse)
                {
                    ((EntityHorse)this.playerEntity.getRidingEntity()).openGUI(this.playerEntity);
                }

                break;

            case START_FALL_FLYING:
                if (!this.playerEntity.onGround && this.playerEntity.motionY < 0.0D && !this.playerEntity.func_184613_cA() && !this.playerEntity.isInWater())
                {
                    ItemStack itemstack = this.playerEntity.getItemStackFromSlot(EntityEquipmentSlot.CHEST);

                    if (itemstack != null && itemstack.getItem() == Items.elytra && ItemElytra.isBroken(itemstack))
                    {
                        this.playerEntity.func_184847_M();
                    }
                }
                else
                {
                    this.playerEntity.func_189103_N();
                }

                break;

            default:
                throw new IllegalArgumentException("Invalid client command!");
        }
    }

    /**
     * Processes interactions ((un)leashing, opening command block GUI) and attacks on an entity with players currently
     * equipped item
     */
    public void processUseEntity(CPacketUseEntity packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);
        Entity entity = packetIn.getEntityFromWorld(worldserver);
        this.playerEntity.markPlayerActive();

        if (entity != null)
        {
            boolean flag = this.playerEntity.canEntityBeSeen(entity);
            double d0 = 36.0D;

            if (!flag)
            {
                d0 = 9.0D;
            }

            if (this.playerEntity.getDistanceSqToEntity(entity) < d0)
            {
                if (packetIn.getAction() == CPacketUseEntity.Action.INTERACT)
                {
                    EnumHand enumhand = packetIn.getHand();
                    ItemStack itemstack = this.playerEntity.getHeldItem(enumhand);
                    this.playerEntity.func_184822_a(entity, itemstack, enumhand);
                }
                else if (packetIn.getAction() == CPacketUseEntity.Action.INTERACT_AT)
                {
                    EnumHand enumhand1 = packetIn.getHand();
                    ItemStack itemstack1 = this.playerEntity.getHeldItem(enumhand1);
                    entity.func_184199_a(this.playerEntity, packetIn.getHitVec(), itemstack1, enumhand1);
                }
                else if (packetIn.getAction() == CPacketUseEntity.Action.ATTACK)
                {
                    if (entity instanceof EntityItem || entity instanceof EntityXPOrb || entity instanceof EntityArrow || entity == this.playerEntity)
                    {
                        this.kickPlayerFromServer("Attempting to attack an invalid entity");
                        this.serverController.logWarning("Player " + this.playerEntity.getName() + " tried to attack an invalid entity");
                        return;
                    }

                    this.playerEntity.attackTargetEntityWithCurrentItem(entity);
                }
            }
        }
    }

    /**
     * Processes the client status updates: respawn attempt from player, opening statistics or achievements, or
     * acquiring 'open inventory' achievement
     */
    public void processClientStatus(CPacketClientStatus packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();
        CPacketClientStatus.State cpacketclientstatus$state = packetIn.getStatus();

        switch (cpacketclientstatus$state)
        {
            case PERFORM_RESPAWN:
                if (this.playerEntity.playerConqueredTheEnd)
                {
                    this.playerEntity.playerConqueredTheEnd = false;
                    this.playerEntity = this.serverController.getPlayerList().recreatePlayerEntity(this.playerEntity, 0, true);
                }
                else
                {
                    if (this.playerEntity.getHealth() > 0.0F)
                    {
                        return;
                    }

                    this.playerEntity = this.serverController.getPlayerList().recreatePlayerEntity(this.playerEntity, 0, false);

                    if (this.serverController.isHardcore())
                    {
                        this.playerEntity.setGameType(WorldSettings.GameType.SPECTATOR);
                        this.playerEntity.getServerForPlayer().getGameRules().setOrCreateGameRule("spectatorsGenerateChunks", "false");
                    }
                }

                break;

            case REQUEST_STATS:
                this.playerEntity.getStatFile().func_150876_a(this.playerEntity);
                break;

            case OPEN_INVENTORY_ACHIEVEMENT:
                this.playerEntity.triggerAchievement(AchievementList.field_187982_f);
        }
    }

    /**
     * Processes the client closing windows (container)
     */
    public void processCloseWindow(CPacketCloseWindow packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.closeContainer();
    }

    /**
     * Executes a container/inventory slot manipulation as indicated by the packet. Sends the serverside result if they
     * didn't match the indicated result and prevents further manipulation by the player until he confirms that it has
     * the same open container/inventory
     */
    public void processClickWindow(CPacketClickWindow packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();

        if (this.playerEntity.openContainer.windowId == packetIn.getWindowId() && this.playerEntity.openContainer.getCanCraft(this.playerEntity))
        {
            if (this.playerEntity.isSpectator())
            {
                List<ItemStack> list = Lists.<ItemStack>newArrayList();

                for (int i = 0; i < this.playerEntity.openContainer.inventorySlots.size(); ++i)
                {
                    list.add(((Slot)this.playerEntity.openContainer.inventorySlots.get(i)).getStack());
                }

                this.playerEntity.updateCraftingInventory(this.playerEntity.openContainer, list);
            }
            else
            {
                ItemStack itemstack2 = this.playerEntity.openContainer.func_184996_a(packetIn.getSlotId(), packetIn.getUsedButton(), packetIn.getClickType(), this.playerEntity);

                if (ItemStack.areItemStacksEqual(packetIn.getClickedItem(), itemstack2))
                {
                    this.playerEntity.playerNetServerHandler.sendPacket(new SPacketConfirmTransaction(packetIn.getWindowId(), packetIn.getActionNumber(), true));
                    this.playerEntity.isChangingQuantityOnly = true;
                    this.playerEntity.openContainer.detectAndSendChanges();
                    this.playerEntity.updateHeldItem();
                    this.playerEntity.isChangingQuantityOnly = false;
                }
                else
                {
                    this.field_147372_n.addKey(this.playerEntity.openContainer.windowId, Short.valueOf(packetIn.getActionNumber()));
                    this.playerEntity.playerNetServerHandler.sendPacket(new SPacketConfirmTransaction(packetIn.getWindowId(), packetIn.getActionNumber(), false));
                    this.playerEntity.openContainer.setCanCraft(this.playerEntity, false);
                    List<ItemStack> list1 = Lists.<ItemStack>newArrayList();

                    for (int j = 0; j < this.playerEntity.openContainer.inventorySlots.size(); ++j)
                    {
                        ItemStack itemstack = ((Slot)this.playerEntity.openContainer.inventorySlots.get(j)).getStack();
                        ItemStack itemstack1 = itemstack != null && itemstack.stackSize > 0 ? itemstack : null;
                        list1.add(itemstack1);
                    }

                    this.playerEntity.updateCraftingInventory(this.playerEntity.openContainer, list1);
                }
            }
        }
    }

    /**
     * Enchants the item identified by the packet given some convoluted conditions (matching window, which
     * should/shouldn't be in use?)
     */
    public void processEnchantItem(CPacketEnchantItem packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();

        if (this.playerEntity.openContainer.windowId == packetIn.getWindowId() && this.playerEntity.openContainer.getCanCraft(this.playerEntity) && !this.playerEntity.isSpectator())
        {
            this.playerEntity.openContainer.enchantItem(this.playerEntity, packetIn.getButton());
            this.playerEntity.openContainer.detectAndSendChanges();
        }
    }

    /**
     * Update the server with an ItemStack in a slot.
     */
    public void processCreativeInventoryAction(CPacketCreativeInventoryAction packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());

        if (this.playerEntity.theItemInWorldManager.isCreative())
        {
            boolean flag = packetIn.getSlotId() < 0;
            ItemStack itemstack = packetIn.getStack();

            if (itemstack != null && itemstack.hasTagCompound() && itemstack.getTagCompound().hasKey("BlockEntityTag", 10))
            {
                NBTTagCompound nbttagcompound = itemstack.getTagCompound().getCompoundTag("BlockEntityTag");

                if (nbttagcompound.hasKey("x") && nbttagcompound.hasKey("y") && nbttagcompound.hasKey("z"))
                {
                    BlockPos blockpos = new BlockPos(nbttagcompound.getInteger("x"), nbttagcompound.getInteger("y"), nbttagcompound.getInteger("z"));
                    TileEntity tileentity = this.playerEntity.worldObj.getTileEntity(blockpos);

                    if (tileentity != null)
                    {
                        NBTTagCompound nbttagcompound1 = new NBTTagCompound();
                        tileentity.writeToNBT(nbttagcompound1);
                        nbttagcompound1.removeTag("x");
                        nbttagcompound1.removeTag("y");
                        nbttagcompound1.removeTag("z");
                        itemstack.setTagInfo("BlockEntityTag", nbttagcompound1);
                    }
                }
            }

            boolean flag1 = packetIn.getSlotId() >= 1 && packetIn.getSlotId() <= 45;
            boolean flag2 = itemstack == null || itemstack.getItem() != null;
            boolean flag3 = itemstack == null || itemstack.getMetadata() >= 0 && itemstack.stackSize <= 64 && itemstack.stackSize > 0;

            if (flag1 && flag2 && flag3)
            {
                if (itemstack == null)
                {
                    this.playerEntity.inventoryContainer.putStackInSlot(packetIn.getSlotId(), (ItemStack)null);
                }
                else
                {
                    this.playerEntity.inventoryContainer.putStackInSlot(packetIn.getSlotId(), itemstack);
                }

                this.playerEntity.inventoryContainer.setCanCraft(this.playerEntity, true);
            }
            else if (flag && flag2 && flag3 && this.itemDropThreshold < 200)
            {
                this.itemDropThreshold += 20;
                EntityItem entityitem = this.playerEntity.dropPlayerItemWithRandomChoice(itemstack, true);

                if (entityitem != null)
                {
                    entityitem.setAgeToCreativeDespawnTime();
                }
            }
        }
    }

    /**
     * Received in response to the server requesting to confirm that the client-side open container matches the servers'
     * after a mismatched container-slot manipulation. It will unlock the player's ability to manipulate the container
     * contents
     */
    public void processConfirmTransaction(CPacketConfirmTransaction packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        Short oshort = (Short)this.field_147372_n.lookup(this.playerEntity.openContainer.windowId);

        if (oshort != null && packetIn.getUid() == oshort.shortValue() && this.playerEntity.openContainer.windowId == packetIn.getWindowId() && !this.playerEntity.openContainer.getCanCraft(this.playerEntity) && !this.playerEntity.isSpectator())
        {
            this.playerEntity.openContainer.setCanCraft(this.playerEntity, true);
        }
    }

    public void processUpdateSign(CPacketUpdateSign packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.markPlayerActive();
        WorldServer worldserver = this.serverController.worldServerForDimension(this.playerEntity.dimension);
        BlockPos blockpos = packetIn.getPosition();

        if (worldserver.isBlockLoaded(blockpos))
        {
            IBlockState iblockstate = worldserver.getBlockState(blockpos);
            TileEntity tileentity = worldserver.getTileEntity(blockpos);

            if (!(tileentity instanceof TileEntitySign))
            {
                return;
            }

            TileEntitySign tileentitysign = (TileEntitySign)tileentity;

            if (!tileentitysign.getIsEditable() || tileentitysign.getPlayer() != this.playerEntity)
            {
                this.serverController.logWarning("Player " + this.playerEntity.getName() + " just tried to change non-editable sign");
                return;
            }

            String[] astring = packetIn.getLines();

            for (int i = 0; i < astring.length; ++i)
            {
                tileentitysign.signText[i] = new TextComponentString(TextFormatting.getTextWithoutFormattingCodes(astring[i]));
            }

            tileentitysign.markDirty();
            worldserver.notifyBlockUpdate(blockpos, iblockstate, iblockstate, 3);
        }
    }

    /**
     * Updates a players' ping statistics
     */
    public void processKeepAlive(CPacketKeepAlive packetIn)
    {
        if (packetIn.getKey() == this.field_147378_h)
        {
            int i = (int)(this.currentTimeMillis() - this.lastPingTime);
            this.playerEntity.ping = (this.playerEntity.ping * 3 + i) / 4;
        }
    }

    private long currentTimeMillis()
    {
        return System.nanoTime() / 1000000L;
    }

    /**
     * Processes a player starting/stopping flying
     */
    public void processPlayerAbilities(CPacketPlayerAbilities packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.capabilities.isFlying = packetIn.isFlying() && this.playerEntity.capabilities.allowFlying;
    }

    /**
     * Retrieves possible tab completions for the requested command string and sends them to the client
     */
    public void processTabComplete(CPacketTabComplete packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        List<String> list = Lists.<String>newArrayList();

        for (String s : this.serverController.getTabCompletions(this.playerEntity, packetIn.getMessage(), packetIn.getTargetBlock(), packetIn.func_186989_c()))
        {
            list.add(s);
        }

        this.playerEntity.playerNetServerHandler.sendPacket(new SPacketTabComplete((String[])list.toArray(new String[list.size()])));
    }

    /**
     * Updates serverside copy of client settings: language, render distance, chat visibility, chat colours, difficulty,
     * and whether to show the cape
     */
    public void processClientSettings(CPacketClientSettings packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        this.playerEntity.handleClientSettings(packetIn);
    }

    /**
     * Synchronizes serverside and clientside book contents and signing
     */
    public void processVanilla250Packet(CPacketCustomPayload packetIn)
    {
        PacketThreadUtil.checkThreadAndEnqueue(packetIn, this, this.playerEntity.getServerForPlayer());
        String s = packetIn.getChannelName();

        if ("MC|BEdit".equals(s))
        {
            PacketBuffer packetbuffer6 = new PacketBuffer(Unpooled.wrappedBuffer((ByteBuf)packetIn.getBufferData()));

            try
            {
                ItemStack itemstack1 = packetbuffer6.readItemStackFromBuffer();

                if (itemstack1 != null)
                {
                    if (!ItemWritableBook.isNBTValid(itemstack1.getTagCompound()))
                    {
                        throw new IOException("Invalid book tag!");
                    }

                    ItemStack itemstack3 = this.playerEntity.getHeldItemMainhand();

                    if (itemstack3 == null)
                    {
                        return;
                    }

                    if (itemstack1.getItem() == Items.writable_book && itemstack1.getItem() == itemstack3.getItem())
                    {
                        itemstack3.setTagInfo("pages", itemstack1.getTagCompound().getTagList("pages", 8));
                    }

                    return;
                }
            }
            catch (Exception exception6)
            {
                logger.error((String)"Couldn\'t handle book info", (Throwable)exception6);
                return;
            }
            finally
            {
                packetbuffer6.release();
            }

            return;
        }
        else if ("MC|BSign".equals(s))
        {
            PacketBuffer packetbuffer5 = new PacketBuffer(Unpooled.wrappedBuffer((ByteBuf)packetIn.getBufferData()));

            try
            {
                ItemStack itemstack = packetbuffer5.readItemStackFromBuffer();

                if (itemstack != null)
                {
                    if (!ItemWrittenBook.validBookTagContents(itemstack.getTagCompound()))
                    {
                        throw new IOException("Invalid book tag!");
                    }

                    ItemStack itemstack2 = this.playerEntity.getHeldItemMainhand();

                    if (itemstack2 == null)
                    {
                        return;
                    }

                    if (itemstack.getItem() == Items.writable_book && itemstack2.getItem() == Items.writable_book)
                    {
                        itemstack2.setTagInfo("author", new NBTTagString(this.playerEntity.getName()));
                        itemstack2.setTagInfo("title", new NBTTagString(itemstack.getTagCompound().getString("title")));
                        NBTTagList nbttaglist = itemstack.getTagCompound().getTagList("pages", 8);

                        for (int j1 = 0; j1 < nbttaglist.tagCount(); ++j1)
                        {
                            String s4 = nbttaglist.getStringTagAt(j1);
                            ITextComponent itextcomponent = new TextComponentString(s4);
                            s4 = ITextComponent.Serializer.componentToJson(itextcomponent);
                            nbttaglist.set(j1, new NBTTagString(s4));
                        }

                        itemstack2.setTagInfo("pages", nbttaglist);
                        itemstack2.setItem(Items.written_book);
                    }

                    return;
                }
            }
            catch (Exception exception7)
            {
                logger.error((String)"Couldn\'t sign book", (Throwable)exception7);
                return;
            }
            finally
            {
                packetbuffer5.release();
            }

            return;
        }
        else if ("MC|TrSel".equals(s))
        {
            try
            {
                int i = packetIn.getBufferData().readInt();
                Container container = this.playerEntity.openContainer;

                if (container instanceof ContainerMerchant)
                {
                    ((ContainerMerchant)container).setCurrentRecipeIndex(i);
                }
            }
            catch (Exception exception5)
            {
                logger.error((String)"Couldn\'t select trade", (Throwable)exception5);
            }
        }
        else if ("MC|AdvCmd".equals(s))
        {
            if (!this.serverController.isCommandBlockEnabled())
            {
                this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.notEnabled", new Object[0]));
                return;
            }

            if (!this.playerEntity.canCommandSenderUseCommand(2, "") || !this.playerEntity.capabilities.isCreativeMode)
            {
                this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.notAllowed", new Object[0]));
                return;
            }

            PacketBuffer packetbuffer = packetIn.getBufferData();

            try
            {
                int j = packetbuffer.readByte();
                CommandBlockBaseLogic commandblockbaselogic = null;

                if (j == 0)
                {
                    TileEntity tileentity = this.playerEntity.worldObj.getTileEntity(new BlockPos(packetbuffer.readInt(), packetbuffer.readInt(), packetbuffer.readInt()));

                    if (tileentity instanceof TileEntityCommandBlock)
                    {
                        commandblockbaselogic = ((TileEntityCommandBlock)tileentity).getCommandBlockLogic();
                    }
                }
                else if (j == 1)
                {
                    Entity entity = this.playerEntity.worldObj.getEntityByID(packetbuffer.readInt());

                    if (entity instanceof EntityMinecartCommandBlock)
                    {
                        commandblockbaselogic = ((EntityMinecartCommandBlock)entity).getCommandBlockLogic();
                    }
                }

                String s3 = packetbuffer.readStringFromBuffer(packetbuffer.readableBytes());
                boolean flag = packetbuffer.readBoolean();

                if (commandblockbaselogic != null)
                {
                    commandblockbaselogic.setCommand(s3);
                    commandblockbaselogic.setTrackOutput(flag);

                    if (!flag)
                    {
                        commandblockbaselogic.setLastOutput((ITextComponent)null);
                    }

                    commandblockbaselogic.updateCommand();
                    this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.setCommand.success", new Object[] {s3}));
                }
            }
            catch (Exception exception4)
            {
                logger.error((String)"Couldn\'t set command block", (Throwable)exception4);
            }
            finally
            {
                packetbuffer.release();
            }
        }
        else if ("MC|AutoCmd".equals(s))
        {
            if (!this.serverController.isCommandBlockEnabled())
            {
                this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.notEnabled", new Object[0]));
                return;
            }

            if (!this.playerEntity.canCommandSenderUseCommand(2, "") || !this.playerEntity.capabilities.isCreativeMode)
            {
                this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.notAllowed", new Object[0]));
                return;
            }

            PacketBuffer packetbuffer1 = packetIn.getBufferData();

            try
            {
                CommandBlockBaseLogic commandblockbaselogic1 = null;
                TileEntityCommandBlock tileentitycommandblock = null;
                BlockPos blockpos1 = new BlockPos(packetbuffer1.readInt(), packetbuffer1.readInt(), packetbuffer1.readInt());
                TileEntity tileentity2 = this.playerEntity.worldObj.getTileEntity(blockpos1);

                if (tileentity2 instanceof TileEntityCommandBlock)
                {
                    tileentitycommandblock = (TileEntityCommandBlock)tileentity2;
                    commandblockbaselogic1 = tileentitycommandblock.getCommandBlockLogic();
                }

                String s1 = packetbuffer1.readStringFromBuffer(packetbuffer1.readableBytes());
                boolean flag1 = packetbuffer1.readBoolean();
                TileEntityCommandBlock.Mode tileentitycommandblock$mode = TileEntityCommandBlock.Mode.valueOf(packetbuffer1.readStringFromBuffer(16));
                boolean flag2 = packetbuffer1.readBoolean();
                boolean flag3 = packetbuffer1.readBoolean();

                if (commandblockbaselogic1 != null)
                {
                    EnumFacing enumfacing = (EnumFacing)this.playerEntity.worldObj.getBlockState(blockpos1).getValue(BlockCommandBlock.FACING);

                    switch (tileentitycommandblock$mode)
                    {
                        case SEQUENCE:
                            IBlockState iblockstate3 = Blocks.chain_command_block.getDefaultState();
                            this.playerEntity.worldObj.setBlockState(blockpos1, iblockstate3.withProperty(BlockCommandBlock.FACING, enumfacing).withProperty(BlockCommandBlock.CONDITIONAL, Boolean.valueOf(flag2)), 2);
                            break;

                        case AUTO:
                            IBlockState iblockstate2 = Blocks.repeating_command_block.getDefaultState();
                            this.playerEntity.worldObj.setBlockState(blockpos1, iblockstate2.withProperty(BlockCommandBlock.FACING, enumfacing).withProperty(BlockCommandBlock.CONDITIONAL, Boolean.valueOf(flag2)), 2);
                            break;

                        case REDSTONE:
                            IBlockState lvt_14_1_ = Blocks.command_block.getDefaultState();
                            this.playerEntity.worldObj.setBlockState(blockpos1, lvt_14_1_.withProperty(BlockCommandBlock.FACING, enumfacing).withProperty(BlockCommandBlock.CONDITIONAL, Boolean.valueOf(flag2)), 2);
                    }

                    tileentity2.validate();
                    this.playerEntity.worldObj.setTileEntity(blockpos1, tileentity2);
                    commandblockbaselogic1.setCommand(s1);
                    commandblockbaselogic1.setTrackOutput(flag1);

                    if (!flag1)
                    {
                        commandblockbaselogic1.setLastOutput((ITextComponent)null);
                    }

                    tileentitycommandblock.setAuto(flag3);
                    commandblockbaselogic1.updateCommand();

                    if (!net.minecraft.util.StringUtils.isNullOrEmpty(s1))
                    {
                        this.playerEntity.addChatMessage(new TextComponentTranslation("advMode.setCommand.success", new Object[] {s1}));
                    }
                }
            }
            catch (Exception exception3)
            {
                logger.error((String)"Couldn\'t set command block", (Throwable)exception3);
            }
            finally
            {
                packetbuffer1.release();
            }
        }
        else if ("MC|Beacon".equals(s))
        {
            if (this.playerEntity.openContainer instanceof ContainerBeacon)
            {
                try
                {
                    PacketBuffer packetbuffer2 = packetIn.getBufferData();
                    int k = packetbuffer2.readInt();
                    int i1 = packetbuffer2.readInt();
                    ContainerBeacon containerbeacon = (ContainerBeacon)this.playerEntity.openContainer;
                    Slot slot = containerbeacon.getSlot(0);

                    if (slot.getHasStack())
                    {
                        slot.decrStackSize(1);
                        IInventory iinventory = containerbeacon.func_180611_e();
                        iinventory.setField(1, k);
                        iinventory.setField(2, i1);
                        iinventory.markDirty();
                    }
                }
                catch (Exception exception2)
                {
                    logger.error((String)"Couldn\'t set beacon", (Throwable)exception2);
                }
            }
        }
        else if ("MC|ItemName".equals(s))
        {
            if (this.playerEntity.openContainer instanceof ContainerRepair)
            {
                ContainerRepair containerrepair = (ContainerRepair)this.playerEntity.openContainer;

                if (packetIn.getBufferData() != null && packetIn.getBufferData().readableBytes() >= 1)
                {
                    String s2 = ChatAllowedCharacters.filterAllowedCharacters(packetIn.getBufferData().readStringFromBuffer(32767));

                    if (s2.length() <= 30)
                    {
                        containerrepair.updateItemName(s2);
                    }
                }
                else
                {
                    containerrepair.updateItemName("");
                }
            }
        }
        else if ("MC|Struct".equals(s))
        {
            PacketBuffer packetbuffer3 = packetIn.getBufferData();

            try
            {
                if (this.playerEntity.canCommandSenderUseCommand(4, "") && this.playerEntity.capabilities.isCreativeMode)
                {
                    BlockPos blockpos = new BlockPos(packetbuffer3.readInt(), packetbuffer3.readInt(), packetbuffer3.readInt());
                    IBlockState iblockstate1 = this.playerEntity.worldObj.getBlockState(blockpos);
                    TileEntity tileentity1 = this.playerEntity.worldObj.getTileEntity(blockpos);

                    if (tileentity1 instanceof TileEntityStructure)
                    {
                        TileEntityStructure tileentitystructure = (TileEntityStructure)tileentity1;
                        int k1 = packetbuffer3.readByte();
                        String s5 = packetbuffer3.readStringFromBuffer(32);
                        tileentitystructure.setMode(TileEntityStructure.Mode.valueOf(s5));
                        tileentitystructure.setName(packetbuffer3.readStringFromBuffer(64));
                        tileentitystructure.setPosition(new BlockPos(packetbuffer3.readInt(), packetbuffer3.readInt(), packetbuffer3.readInt()));
                        tileentitystructure.setSize(new BlockPos(packetbuffer3.readInt(), packetbuffer3.readInt(), packetbuffer3.readInt()));
                        String s6 = packetbuffer3.readStringFromBuffer(32);
                        tileentitystructure.setMirror(Mirror.valueOf(s6));
                        String s7 = packetbuffer3.readStringFromBuffer(32);
                        tileentitystructure.setRotation(Rotation.valueOf(s7));
                        tileentitystructure.setMetadata(packetbuffer3.readStringFromBuffer(128));
                        tileentitystructure.setIgnoresEntities(packetbuffer3.readBoolean());

                        if (k1 == 2)
                        {
                            if (tileentitystructure.func_184419_m())
                            {
                                this.playerEntity.addChatComponentMessage(new TextComponentString("Structure saved"));
                            }
                            else
                            {
                                this.playerEntity.addChatComponentMessage(new TextComponentString("Structure NOT saved"));
                            }
                        }
                        else if (k1 == 3)
                        {
                            if (tileentitystructure.func_184412_n())
                            {
                                this.playerEntity.addChatComponentMessage(new TextComponentString("Structure loaded"));
                            }
                            else
                            {
                                this.playerEntity.addChatComponentMessage(new TextComponentString("Structure prepared"));
                            }
                        }
                        else if (k1 == 4 && tileentitystructure.func_184417_l())
                        {
                            this.playerEntity.addChatComponentMessage(new TextComponentString("Size detected"));
                        }

                        tileentitystructure.markDirty();
                        this.playerEntity.worldObj.notifyBlockUpdate(blockpos, iblockstate1, iblockstate1, 3);
                    }
                }
            }
            catch (Exception exception1)
            {
                logger.error((String)"Couldn\'t set structure block", (Throwable)exception1);
            }
            finally
            {
                packetbuffer3.release();
            }
        }
        else if ("MC|PickItem".equals(s))
        {
            PacketBuffer packetbuffer4 = packetIn.getBufferData();

            try
            {
                int l = packetbuffer4.readVarIntFromBuffer();
                this.playerEntity.inventory.func_184430_d(l);
                this.playerEntity.playerNetServerHandler.sendPacket(new SPacketSetSlot(-2, this.playerEntity.inventory.currentItem, this.playerEntity.inventory.getStackInSlot(this.playerEntity.inventory.currentItem)));
                this.playerEntity.playerNetServerHandler.sendPacket(new SPacketSetSlot(-2, l, this.playerEntity.inventory.getStackInSlot(l)));
                this.playerEntity.playerNetServerHandler.sendPacket(new SPacketHeldItemChange(this.playerEntity.inventory.currentItem));
            }
            catch (Exception exception)
            {
                logger.error((String)"Couldn\'t pick item", (Throwable)exception);
            }
            finally
            {
                packetbuffer4.release();
            }
        }
    }
}
