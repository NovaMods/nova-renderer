package net.minecraft.world.gen.structure.template;

import net.minecraft.block.Block;
import net.minecraft.util.Mirror;
import net.minecraft.util.Rotation;
import net.minecraft.world.ChunkCoordIntPair;
import net.minecraft.world.gen.structure.StructureBoundingBox;

public class PlacementSettings
{
    private Mirror field_186228_a;
    private Rotation field_186229_b;
    private boolean field_186230_c;
    private Block field_186231_d;
    private ChunkCoordIntPair field_186232_e;
    private StructureBoundingBox field_186233_f;
    private boolean field_186234_g;

    public PlacementSettings()
    {
        this(Mirror.NONE, Rotation.NONE, false, (Block)null, (StructureBoundingBox)null);
    }

    public PlacementSettings(Mirror p_i47087_1_, Rotation p_i47087_2_, boolean p_i47087_3_, Block p_i47087_4_, StructureBoundingBox p_i47087_5_)
    {
        this.field_186229_b = p_i47087_2_;
        this.field_186228_a = p_i47087_1_;
        this.field_186230_c = p_i47087_3_;
        this.field_186231_d = p_i47087_4_;
        this.field_186232_e = null;
        this.field_186233_f = p_i47087_5_;
        this.field_186234_g = true;
    }

    public PlacementSettings func_186217_a()
    {
        return (new PlacementSettings(this.field_186228_a, this.field_186229_b, this.field_186230_c, this.field_186231_d, this.field_186233_f)).func_186218_a(this.field_186232_e).func_186226_b(this.field_186234_g);
    }

    public PlacementSettings func_186214_a(Mirror p_186214_1_)
    {
        this.field_186228_a = p_186214_1_;
        return this;
    }

    public PlacementSettings func_186220_a(Rotation p_186220_1_)
    {
        this.field_186229_b = p_186220_1_;
        return this;
    }

    public PlacementSettings func_186222_a(boolean p_186222_1_)
    {
        this.field_186230_c = p_186222_1_;
        return this;
    }

    public PlacementSettings func_186225_a(Block p_186225_1_)
    {
        this.field_186231_d = p_186225_1_;
        return this;
    }

    public PlacementSettings func_186218_a(ChunkCoordIntPair p_186218_1_)
    {
        this.field_186232_e = p_186218_1_;
        return this;
    }

    public PlacementSettings func_186223_a(StructureBoundingBox p_186223_1_)
    {
        this.field_186233_f = p_186223_1_;
        return this;
    }

    public Mirror func_186212_b()
    {
        return this.field_186228_a;
    }

    public PlacementSettings func_186226_b(boolean p_186226_1_)
    {
        this.field_186234_g = p_186226_1_;
        return this;
    }

    public Rotation func_186215_c()
    {
        return this.field_186229_b;
    }

    public boolean func_186221_e()
    {
        return this.field_186230_c;
    }

    public Block func_186219_f()
    {
        return this.field_186231_d;
    }

    public StructureBoundingBox func_186213_g()
    {
        if (this.field_186233_f == null && this.field_186232_e != null)
        {
            this.func_186224_i();
        }

        return this.field_186233_f;
    }

    public boolean func_186227_h()
    {
        return this.field_186234_g;
    }

    void func_186224_i()
    {
        this.field_186233_f = this.func_186216_b(this.field_186232_e);
    }

    private StructureBoundingBox func_186216_b(ChunkCoordIntPair p_186216_1_)
    {
        if (p_186216_1_ == null)
        {
            return null;
        }
        else
        {
            int i = p_186216_1_.chunkXPos * 16;
            int j = p_186216_1_.chunkZPos * 16;
            return new StructureBoundingBox(i, 0, j, i + 16 - 1, 255, j + 16 - 1);
        }
    }
}
