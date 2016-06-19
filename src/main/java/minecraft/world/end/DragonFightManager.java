package net.minecraft.world.end;

import com.google.common.base.Predicate;
import com.google.common.base.Predicates;
import com.google.common.collect.ContiguousSet;
import com.google.common.collect.DiscreteDomain;
import com.google.common.collect.Lists;
import com.google.common.collect.Range;
import com.google.common.collect.Sets;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.UUID;
import net.minecraft.block.state.BlockWorldState;
import net.minecraft.block.state.pattern.BlockMatcher;
import net.minecraft.block.state.pattern.BlockPattern;
import net.minecraft.block.state.pattern.FactoryBlockPattern;
import net.minecraft.entity.Entity;
import net.minecraft.entity.boss.EntityDragon;
import net.minecraft.entity.boss.dragon.phase.PhaseList;
import net.minecraft.entity.item.EntityEnderCrystal;
import net.minecraft.entity.player.EntityPlayerMP;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTTagInt;
import net.minecraft.nbt.NBTTagList;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityEndPortal;
import net.minecraft.util.DamageSource;
import net.minecraft.util.EntitySelectors;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.math.AxisAlignedBB;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.text.TextComponentTranslation;
import net.minecraft.world.BossInfo;
import net.minecraft.world.BossInfoServer;
import net.minecraft.world.WorldServer;
import net.minecraft.world.biome.BiomeEndDecorator;
import net.minecraft.world.chunk.Chunk;
import net.minecraft.world.gen.feature.WorldGenEndGateway;
import net.minecraft.world.gen.feature.WorldGenEndPodium;
import net.minecraft.world.gen.feature.WorldGenSpikes;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class DragonFightManager
{
    private static final Logger LOGGER = LogManager.getLogger();
    private static final Predicate<EntityPlayerMP> field_186108_b = Predicates.<EntityPlayerMP> and (EntitySelectors.selectAnything, EntitySelectors.<EntityPlayerMP>func_188443_a(0.0D, 128.0D, 0.0D, 192.0D));
    private final BossInfoServer bossInfo = (BossInfoServer)(new BossInfoServer(new TextComponentTranslation("entity.EnderDragon.name", new Object[0]), BossInfo.Color.PINK, BossInfo.Overlay.PROGRESS)).setPlayEndBossMusic(true).setCreateFog(true);
    private final WorldServer world;
    private final List<Integer> field_186111_e = Lists.<Integer>newArrayList();
    private final BlockPattern field_186112_f;
    private int field_186113_g = 0;
    private int field_186114_h = 0;
    private int field_186115_i = 0;
    private int field_186116_j = 0;
    private boolean dragonKilled = false;
    private boolean previouslyKilled = false;
    private UUID dragonUniqueId = null;
    private boolean field_186120_n = false;
    private BlockPos exitPortalLocation = null;
    private DragonSpawnManager respawnState;
    private int field_186123_q = 0;
    private List<EntityEnderCrystal> field_186124_r;

    public DragonFightManager(WorldServer worldIn, NBTTagCompound compound)
    {
        this.world = worldIn;

        if (compound.hasKey("DragonKilled", 1))
        {
            if (compound.hasUniqueId("DragonUUID"))
            {
                this.dragonUniqueId = compound.getUniqueId("DragonUUID");
            }

            this.dragonKilled = compound.getBoolean("DragonKilled");
            this.previouslyKilled = compound.getBoolean("PreviouslyKilled");

            if (compound.getBoolean("IsRespawning"))
            {
                this.respawnState = DragonSpawnManager.START;
            }

            if (compound.hasKey("ExitPortalLocation", 10))
            {
                this.exitPortalLocation = NBTUtil.getPosFromTag(compound.getCompoundTag("ExitPortalLocation"));
            }
        }
        else
        {
            this.field_186120_n = true;
            this.dragonKilled = true;
            this.previouslyKilled = true;
        }

        if (compound.hasKey("Gateways", 9))
        {
            NBTTagList nbttaglist = compound.getTagList("Gateways", 3);

            for (int i = 0; i < nbttaglist.tagCount(); ++i)
            {
                this.field_186111_e.add(Integer.valueOf(nbttaglist.getIntAt(i)));
            }
        }
        else
        {
            this.field_186111_e.addAll(ContiguousSet.<Integer>create(Range.<Integer>closedOpen(Integer.valueOf(0), Integer.valueOf(20)), DiscreteDomain.integers()));
            Collections.shuffle(this.field_186111_e, new Random(worldIn.getSeed()));
        }

        this.field_186112_f = FactoryBlockPattern.start().aisle(new String[] {"       ", "       ", "       ", "   #   ", "       ", "       ", "       "}).aisle(new String[] {"       ", "       ", "       ", "   #   ", "       ", "       ", "       "}).aisle(new String[] {"       ", "       ", "       ", "   #   ", "       ", "       ", "       "}).aisle(new String[] {"  ###  ", " #   # ", "#     #", "#  #  #", "#     #", " #   # ", "  ###  "}).aisle(new String[] {"       ", "  ###  ", " ##### ", " ##### ", " ##### ", "  ###  ", "       "}).where('#', BlockWorldState.hasState(BlockMatcher.forBlock(Blocks.bedrock))).build();
    }

    public NBTTagCompound func_186088_a()
    {
        if (this.field_186120_n)
        {
            return new NBTTagCompound();
        }
        else
        {
            NBTTagCompound nbttagcompound = new NBTTagCompound();

            if (this.dragonUniqueId != null)
            {
                nbttagcompound.setUniqueId("DragonUUID", this.dragonUniqueId);
            }

            nbttagcompound.setBoolean("DragonKilled", this.dragonKilled);
            nbttagcompound.setBoolean("PreviouslyKilled", this.previouslyKilled);

            if (this.exitPortalLocation != null)
            {
                nbttagcompound.setTag("ExitPortalLocation", NBTUtil.createPosTag(this.exitPortalLocation));
            }

            NBTTagList nbttaglist = new NBTTagList();
            Iterator iterator = this.field_186111_e.iterator();

            while (iterator.hasNext())
            {
                int i = ((Integer)iterator.next()).intValue();
                nbttaglist.appendTag(new NBTTagInt(i));
            }

            nbttagcompound.setTag("Gateways", nbttaglist);
            return nbttagcompound;
        }
    }

    public void func_186105_b()
    {
        this.bossInfo.setVisible(!this.dragonKilled);

        if (++this.field_186116_j >= 20)
        {
            this.func_186100_j();
            this.field_186116_j = 0;
        }

        if (!this.bossInfo.getPlayers().isEmpty())
        {
            if (this.field_186120_n)
            {
                LOGGER.info("Scanning for legacy world dragon fight...");
                this.func_186103_i();
                this.field_186120_n = false;

                if (this.func_186104_g())
                {
                    LOGGER.info("Found that the dragon has been killed in this world already.");
                    this.previouslyKilled = true;
                }
                else
                {
                    LOGGER.info("Found that the dragon has not yet been killed in this world.");
                    this.previouslyKilled = false;
                }

                List<EntityDragon> list = this.world.getEntities(EntityDragon.class, EntitySelectors.selectAnything);

                if (!list.isEmpty())
                {
                    EntityDragon entitydragon = (EntityDragon)list.get(0);
                    this.dragonUniqueId = entitydragon.getUniqueID();
                    LOGGER.info("Found that there\'s a dragon still alive (" + entitydragon + ")");
                    this.dragonKilled = false;
                }
                else
                {
                    this.dragonKilled = true;
                }

                if (!this.previouslyKilled && this.dragonKilled)
                {
                    this.dragonKilled = false;
                }
            }

            if (this.respawnState != null)
            {
                if (this.field_186124_r == null)
                {
                    this.respawnState = null;
                    this.func_186106_e();
                }

                this.respawnState.func_186079_a(this.world, this, this.field_186124_r, this.field_186123_q++, this.exitPortalLocation);
            }

            if (!this.dragonKilled)
            {
                if (this.dragonUniqueId == null || ++this.field_186113_g >= 1200)
                {
                    this.func_186103_i();
                    List<EntityDragon> list1 = this.world.getEntities(EntityDragon.class, EntitySelectors.selectAnything);

                    if (!list1.isEmpty())
                    {
                        LOGGER.debug("Haven\'t seen our dragon, but found another one to use.");
                        this.dragonUniqueId = ((EntityDragon)list1.get(0)).getUniqueID();
                    }
                    else
                    {
                        LOGGER.debug("Haven\'t seen the dragon, respawning it");
                        this.spawnDragon();
                        this.func_186094_a(false);
                    }

                    this.field_186113_g = 0;
                }

                if (++this.field_186115_i >= 100)
                {
                    this.func_186101_k();
                    this.field_186115_i = 0;
                }
            }
        }
    }

    protected void func_186095_a(DragonSpawnManager state)
    {
        if (this.respawnState == null)
        {
            throw new IllegalStateException("Dragon respawn isn\'t in progress, can\'t skip ahead in the animation.");
        }
        else
        {
            this.field_186123_q = 0;

            if (state == DragonSpawnManager.END)
            {
                this.respawnState = null;
                this.dragonKilled = false;
                this.spawnDragon();
            }
            else
            {
                this.respawnState = state;
            }
        }
    }

    private boolean func_186104_g()
    {
        for (int i = -8; i <= 8; ++i)
        {
            for (int j = -8; j <= 8; ++j)
            {
                Chunk chunk = this.world.getChunkFromChunkCoords(i, j);

                for (TileEntity tileentity : chunk.getTileEntityMap().values())
                {
                    if (tileentity instanceof TileEntityEndPortal)
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    private BlockPattern.PatternHelper func_186091_h()
    {
        for (int i = -8; i <= 8; ++i)
        {
            for (int j = -8; j <= 8; ++j)
            {
                Chunk chunk = this.world.getChunkFromChunkCoords(i, j);

                for (TileEntity tileentity : chunk.getTileEntityMap().values())
                {
                    if (tileentity instanceof TileEntityEndPortal)
                    {
                        BlockPattern.PatternHelper blockpattern$patternhelper = this.field_186112_f.match(this.world, tileentity.getPos());

                        if (blockpattern$patternhelper != null)
                        {
                            BlockPos blockpos = blockpattern$patternhelper.translateOffset(3, 3, 4).getPos();

                            if (this.exitPortalLocation == null && blockpos.getX() == 0 && blockpos.getZ() == 0)
                            {
                                this.exitPortalLocation = blockpos;
                            }

                            return blockpattern$patternhelper;
                        }
                    }
                }
            }
        }

        int k = this.world.getHeight(WorldGenEndPodium.field_186139_a).getY();

        for (int l = k; l >= 0; --l)
        {
            BlockPattern.PatternHelper blockpattern$patternhelper1 = this.field_186112_f.match(this.world, new BlockPos(WorldGenEndPodium.field_186139_a.getX(), l, WorldGenEndPodium.field_186139_a.getZ()));

            if (blockpattern$patternhelper1 != null)
            {
                if (this.exitPortalLocation == null)
                {
                    this.exitPortalLocation = blockpattern$patternhelper1.translateOffset(3, 3, 4).getPos();
                }

                return blockpattern$patternhelper1;
            }
        }

        return null;
    }

    private void func_186103_i()
    {
        for (int i = -8; i <= 8; ++i)
        {
            for (int j = -8; j <= 8; ++j)
            {
                this.world.getChunkFromChunkCoords(i, j);
            }
        }
    }

    private void func_186100_j()
    {
        Set<EntityPlayerMP> set = Sets.<EntityPlayerMP>newHashSet();

        for (EntityPlayerMP entityplayermp : this.world.getPlayers(EntityPlayerMP.class, field_186108_b))
        {
            this.bossInfo.addPlayer(entityplayermp);
            set.add(entityplayermp);
        }

        Set<EntityPlayerMP> set1 = Sets.newHashSet(this.bossInfo.getPlayers());
        set1.removeAll(set);

        for (EntityPlayerMP entityplayermp1 : set1)
        {
            this.bossInfo.removePlayer(entityplayermp1);
        }
    }

    private void func_186101_k()
    {
        this.field_186115_i = 0;
        this.field_186114_h = 0;

        for (WorldGenSpikes.EndSpike worldgenspikes$endspike : BiomeEndDecorator.func_185426_a(this.world))
        {
            this.field_186114_h += this.world.getEntitiesWithinAABB(EntityEnderCrystal.class, worldgenspikes$endspike.func_186153_f()).size();
        }

        LOGGER.debug("Found {} end crystals still alive", new Object[] {Integer.valueOf(this.field_186114_h)});
    }

    public void processDragonDeath(EntityDragon dragon)
    {
        if (dragon.getUniqueID().equals(this.dragonUniqueId))
        {
            this.bossInfo.setPercent(0.0F);
            this.bossInfo.setVisible(false);
            this.func_186094_a(true);
            this.func_186097_l();

            if (!this.previouslyKilled)
            {
                this.world.setBlockState(this.world.getHeight(WorldGenEndPodium.field_186139_a), Blocks.dragon_egg.getDefaultState());
            }

            this.previouslyKilled = true;
            this.dragonKilled = true;
        }
    }

    private void func_186097_l()
    {
        if (!this.field_186111_e.isEmpty())
        {
            int i = ((Integer)this.field_186111_e.remove(this.field_186111_e.size() - 1)).intValue();
            int j = (int)(96.0D * Math.cos(2.0D * (-Math.PI + 0.15707963267948966D * (double)i)));
            int k = (int)(96.0D * Math.sin(2.0D * (-Math.PI + 0.15707963267948966D * (double)i)));
            this.func_186089_a(new BlockPos(j, 75, k));
        }
    }

    private void func_186089_a(BlockPos p_186089_1_)
    {
        this.world.playAuxSFX(3000, p_186089_1_, 0);
        (new WorldGenEndGateway()).generate(this.world, new Random(), p_186089_1_);
    }

    private void func_186094_a(boolean p_186094_1_)
    {
        WorldGenEndPodium worldgenendpodium = new WorldGenEndPodium(p_186094_1_);

        if (this.exitPortalLocation == null)
        {
            for (this.exitPortalLocation = this.world.getTopSolidOrLiquidBlock(WorldGenEndPodium.field_186139_a).down(); this.world.getBlockState(this.exitPortalLocation).getBlock() == Blocks.bedrock && this.exitPortalLocation.getY() > this.world.getSeaLevel(); this.exitPortalLocation = this.exitPortalLocation.down())
            {
                ;
            }
        }

        worldgenendpodium.generate(this.world, new Random(), this.exitPortalLocation);
    }

    private void spawnDragon()
    {
        this.world.getChunkFromBlockCoords(new BlockPos(0, 128, 0));
        EntityDragon entitydragon = new EntityDragon(this.world);
        entitydragon.getPhaseManager().func_188758_a(PhaseList.HOLDING_PATTERN);
        entitydragon.setLocationAndAngles(0.0D, 128.0D, 0.0D, this.world.rand.nextFloat() * 360.0F, 0.0F);
        this.world.spawnEntityInWorld(entitydragon);
        this.dragonUniqueId = entitydragon.getUniqueID();
    }

    public void func_186099_b(EntityDragon p_186099_1_)
    {
        if (p_186099_1_.getUniqueID().equals(this.dragonUniqueId))
        {
            this.bossInfo.setPercent(p_186099_1_.getHealth() / p_186099_1_.getMaxHealth());
            this.field_186113_g = 0;
        }
    }

    public int func_186092_c()
    {
        return this.field_186114_h;
    }

    public void func_186090_a(EntityEnderCrystal p_186090_1_, DamageSource p_186090_2_)
    {
        if (this.respawnState != null && this.field_186124_r.contains(p_186090_1_))
        {
            LOGGER.debug("Aborting respawn sequence");
            this.respawnState = null;
            this.field_186123_q = 0;
            this.func_186087_f();
            this.func_186094_a(true);
        }
        else
        {
            this.func_186101_k();
            Entity entity = this.world.getEntityFromUuid(this.dragonUniqueId);

            if (entity instanceof EntityDragon)
            {
                ((EntityDragon)entity).func_184672_a(p_186090_1_, new BlockPos(p_186090_1_), p_186090_2_);
            }
        }
    }

    public boolean func_186102_d()
    {
        return this.previouslyKilled;
    }

    public void func_186106_e()
    {
        if (this.dragonKilled && this.respawnState == null)
        {
            BlockPos blockpos = this.exitPortalLocation;

            if (blockpos == null)
            {
                LOGGER.debug("Tried to respawn, but need to find the portal first.");
                BlockPattern.PatternHelper blockpattern$patternhelper = this.func_186091_h();

                if (blockpattern$patternhelper == null)
                {
                    LOGGER.debug("Couldn\'t find a portal, so we made one.");
                    this.func_186094_a(true);
                    blockpos = this.exitPortalLocation;
                }
                else
                {
                    LOGGER.debug("Found the exit portal & temporarily using it.");
                    blockpos = blockpattern$patternhelper.translateOffset(3, 3, 3).getPos();
                }
            }

            List<EntityEnderCrystal> list1 = Lists.<EntityEnderCrystal>newArrayList();
            BlockPos blockpos1 = blockpos.up(1);

            for (EnumFacing enumfacing : EnumFacing.Plane.HORIZONTAL)
            {
                List<EntityEnderCrystal> list = this.world.getEntitiesWithinAABB(EntityEnderCrystal.class, new AxisAlignedBB(blockpos1.offset(enumfacing, 2)));

                if (list.isEmpty())
                {
                    return;
                }

                list1.addAll(list);
            }

            LOGGER.debug("Found all crystals, respawning dragon.");
            this.func_186093_a(list1);
        }
    }

    private void func_186093_a(List<EntityEnderCrystal> p_186093_1_)
    {
        if (this.dragonKilled && this.respawnState == null)
        {
            for (BlockPattern.PatternHelper blockpattern$patternhelper = this.func_186091_h(); blockpattern$patternhelper != null; blockpattern$patternhelper = this.func_186091_h())
            {
                for (int i = 0; i < this.field_186112_f.getPalmLength(); ++i)
                {
                    for (int j = 0; j < this.field_186112_f.getThumbLength(); ++j)
                    {
                        for (int k = 0; k < this.field_186112_f.func_185922_a(); ++k)
                        {
                            BlockWorldState blockworldstate = blockpattern$patternhelper.translateOffset(i, j, k);

                            if (blockworldstate.getBlockState().getBlock() == Blocks.bedrock || blockworldstate.getBlockState().getBlock() == Blocks.end_portal)
                            {
                                this.world.setBlockState(blockworldstate.getPos(), Blocks.end_stone.getDefaultState());
                            }
                        }
                    }
                }
            }

            this.respawnState = DragonSpawnManager.START;
            this.field_186123_q = 0;
            this.func_186094_a(false);
            this.field_186124_r = p_186093_1_;
        }
    }

    public void func_186087_f()
    {
        for (WorldGenSpikes.EndSpike worldgenspikes$endspike : BiomeEndDecorator.func_185426_a(this.world))
        {
            for (EntityEnderCrystal entityendercrystal : this.world.getEntitiesWithinAABB(EntityEnderCrystal.class, worldgenspikes$endspike.func_186153_f()))
            {
                entityendercrystal.func_184224_h(false);
                entityendercrystal.setBeamTarget((BlockPos)null);
            }
        }
    }
}
