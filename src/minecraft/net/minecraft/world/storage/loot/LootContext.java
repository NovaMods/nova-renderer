package net.minecraft.world.storage.loot;

import com.google.common.collect.Sets;
import com.google.gson.TypeAdapter;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonWriter;
import java.io.IOException;
import java.util.Set;
import net.minecraft.entity.Entity;
import net.minecraft.entity.player.EntityPlayer;
import net.minecraft.util.DamageSource;
import net.minecraft.world.WorldServer;

public class LootContext
{
    private final float luck;
    private final WorldServer worldObj;
    private final LootTableManager field_186500_c;
    private final Entity lootedEntity;
    private final EntityPlayer player;
    private final DamageSource damageSource;
    private final Set<LootTable> lootTables = Sets.<LootTable>newLinkedHashSet();

    public LootContext(float p_i46640_1_, WorldServer p_i46640_2_, LootTableManager p_i46640_3_, Entity p_i46640_4_, EntityPlayer p_i46640_5_, DamageSource p_i46640_6_)
    {
        this.luck = p_i46640_1_;
        this.worldObj = p_i46640_2_;
        this.field_186500_c = p_i46640_3_;
        this.lootedEntity = p_i46640_4_;
        this.player = p_i46640_5_;
        this.damageSource = p_i46640_6_;
    }

    public Entity getLootedEntity()
    {
        return this.lootedEntity;
    }

    public Entity getKillerPlayer()
    {
        return this.player;
    }

    public Entity getKiller()
    {
        return this.damageSource == null ? null : this.damageSource.getEntity();
    }

    public boolean addLootTable(LootTable p_186496_1_)
    {
        return this.lootTables.add(p_186496_1_);
    }

    public void removeLootTable(LootTable p_186490_1_)
    {
        this.lootTables.remove(p_186490_1_);
    }

    public LootTableManager getLootTableManager()
    {
        return this.field_186500_c;
    }

    public float getLuck()
    {
        return this.luck;
    }

    public Entity getEntity(LootContext.EntityTarget p_186494_1_)
    {
        switch (p_186494_1_)
        {
            case THIS:
                return this.getLootedEntity();

            case KILLER:
                return this.getKiller();

            case KILLER_PLAYER:
                return this.getKillerPlayer();

            default:
                return null;
        }
    }

    public static class Builder
    {
        private final WorldServer field_186474_a;
        private float field_186475_b;
        private Entity field_186476_c;
        private EntityPlayer field_186477_d;
        private DamageSource field_186478_e;

        public Builder(WorldServer p_i46993_1_)
        {
            this.field_186474_a = p_i46993_1_;
        }

        public LootContext.Builder withLuck(float p_186469_1_)
        {
            this.field_186475_b = p_186469_1_;
            return this;
        }

        public LootContext.Builder withLootedEntity(Entity p_186472_1_)
        {
            this.field_186476_c = p_186472_1_;
            return this;
        }

        public LootContext.Builder withPlayer(EntityPlayer p_186470_1_)
        {
            this.field_186477_d = p_186470_1_;
            return this;
        }

        public LootContext.Builder withDamageSource(DamageSource p_186473_1_)
        {
            this.field_186478_e = p_186473_1_;
            return this;
        }

        public LootContext build()
        {
            return new LootContext(this.field_186475_b, this.field_186474_a, this.field_186474_a.getLootTableManager(), this.field_186476_c, this.field_186477_d, this.field_186478_e);
        }
    }

    public static enum EntityTarget
    {
        THIS("this"),
        KILLER("killer"),
        KILLER_PLAYER("killer_player");

        private final String field_186488_d;

        private EntityTarget(String p_i46992_3_)
        {
            this.field_186488_d = p_i46992_3_;
        }

        public static LootContext.EntityTarget func_186482_a(String p_186482_0_)
        {
            for (LootContext.EntityTarget lootcontext$entitytarget : values())
            {
                if (lootcontext$entitytarget.field_186488_d.equals(p_186482_0_))
                {
                    return lootcontext$entitytarget;
                }
            }

            throw new IllegalArgumentException("Invalid entity target " + p_186482_0_);
        }

        public static class Serializer extends TypeAdapter<LootContext.EntityTarget> {
            public void write(JsonWriter p_write_1_, LootContext.EntityTarget p_write_2_) throws IOException {
                p_write_1_.value(p_write_2_.field_186488_d);
            }

            public LootContext.EntityTarget read(JsonReader p_read_1_) throws IOException {
                return LootContext.EntityTarget.func_186482_a(p_read_1_.nextString());
            }
        }
    }
}
