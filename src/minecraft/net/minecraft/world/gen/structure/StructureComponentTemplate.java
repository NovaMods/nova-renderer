package net.minecraft.world.gen.structure;

import java.util.Map;
import java.util.Random;
import net.minecraft.init.Blocks;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.Rotation;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.gen.structure.template.PlacementSettings;
import net.minecraft.world.gen.structure.template.Template;

public abstract class StructureComponentTemplate extends StructureComponent
{
    private static final PlacementSettings field_186179_d = new PlacementSettings();
    protected Template field_186176_a;
    protected PlacementSettings field_186177_b;
    protected BlockPos field_186178_c;

    public StructureComponentTemplate()
    {
        this.field_186177_b = field_186179_d.func_186222_a(true).func_186225_a(Blocks.air);
    }

    public StructureComponentTemplate(int p_i46662_1_)
    {
        super(p_i46662_1_);
        this.field_186177_b = field_186179_d.func_186222_a(true).func_186225_a(Blocks.air);
    }

    protected void func_186173_a(Template p_186173_1_, BlockPos p_186173_2_, PlacementSettings p_186173_3_)
    {
        this.field_186176_a = p_186173_1_;
        this.func_186164_a(EnumFacing.NORTH);
        this.field_186178_c = p_186173_2_;
        this.field_186177_b = p_186173_3_;
        this.func_186174_h();
    }

    /**
     * (abstract) Helper method to write subclass data to NBT
     */
    protected void writeStructureToNBT(NBTTagCompound tagCompound)
    {
        tagCompound.setInteger("TPX", this.field_186178_c.getX());
        tagCompound.setInteger("TPY", this.field_186178_c.getY());
        tagCompound.setInteger("TPZ", this.field_186178_c.getZ());
    }

    /**
     * (abstract) Helper method to read subclass data from NBT
     */
    protected void readStructureFromNBT(NBTTagCompound tagCompound)
    {
        this.field_186178_c = new BlockPos(tagCompound.getInteger("TPX"), tagCompound.getInteger("TPY"), tagCompound.getInteger("TPZ"));
    }

    /**
     * second Part of Structure generating, this for example places Spiderwebs, Mob Spawners, it closes Mineshafts at
     * the end, it adds Fences...
     */
    public boolean addComponentParts(World worldIn, Random randomIn, StructureBoundingBox structureBoundingBoxIn)
    {
        this.field_186177_b.func_186223_a(structureBoundingBoxIn);
        this.field_186176_a.func_186253_b(worldIn, this.field_186178_c, this.field_186177_b);
        Map<BlockPos, String> map = this.field_186176_a.func_186258_a(this.field_186178_c, this.field_186177_b);

        for (BlockPos blockpos : map.keySet())
        {
            String s = (String)map.get(blockpos);
            this.func_186175_a(s, blockpos, worldIn, randomIn, structureBoundingBoxIn);
        }

        return true;
    }

    protected abstract void func_186175_a(String p_186175_1_, BlockPos p_186175_2_, World p_186175_3_, Random p_186175_4_, StructureBoundingBox p_186175_5_);

    private void func_186174_h()
    {
        Rotation rotation = this.field_186177_b.func_186215_c();
        BlockPos blockpos = this.field_186176_a.func_186257_a(rotation);
        this.boundingBox = new StructureBoundingBox(0, 0, 0, blockpos.getX(), blockpos.getY() - 1, blockpos.getZ());

        switch (rotation)
        {
            case NONE:
            default:
                break;

            case CLOCKWISE_90:
                this.boundingBox.offset(-blockpos.getX(), 0, 0);
                break;

            case COUNTERCLOCKWISE_90:
                this.boundingBox.offset(0, 0, -blockpos.getZ());
                break;

            case CLOCKWISE_180:
                this.boundingBox.offset(-blockpos.getX(), 0, -blockpos.getZ());
        }

        this.boundingBox.offset(this.field_186178_c.getX(), this.field_186178_c.getY(), this.field_186178_c.getZ());
    }

    public void func_181138_a(int p_181138_1_, int p_181138_2_, int p_181138_3_)
    {
        super.func_181138_a(p_181138_1_, p_181138_2_, p_181138_3_);
        this.field_186178_c = this.field_186178_c.add(p_181138_1_, p_181138_2_, p_181138_3_);
    }
}
