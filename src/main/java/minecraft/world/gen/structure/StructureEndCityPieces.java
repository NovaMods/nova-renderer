package net.minecraft.world.gen.structure;

import com.google.common.collect.Lists;
import java.util.List;
import java.util.Random;
import net.minecraft.entity.item.EntityItemFrame;
import net.minecraft.entity.monster.EntityShulker;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.server.MinecraftServer;
import net.minecraft.tileentity.TileEntity;
import net.minecraft.tileentity.TileEntityChest;
import net.minecraft.util.EnumFacing;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.Rotation;
import net.minecraft.util.Tuple;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.World;
import net.minecraft.world.gen.structure.template.PlacementSettings;
import net.minecraft.world.gen.structure.template.Template;
import net.minecraft.world.gen.structure.template.TemplateManager;
import net.minecraft.world.storage.loot.LootTableList;

public class StructureEndCityPieces
{
    public static final TemplateManager field_186201_a = new TemplateManager();
    private static final PlacementSettings field_186202_b = (new PlacementSettings()).func_186222_a(true);
    private static final PlacementSettings field_186203_c = (new PlacementSettings()).func_186222_a(true).func_186225_a(Blocks.air);
    private static final StructureEndCityPieces.IGenerator field_186204_d = new StructureEndCityPieces.IGenerator()
    {
        public void func_186184_a()
        {
        }
        public boolean func_186185_a(int p_186185_1_, StructureEndCityPieces.CityTemplate p_186185_2_, BlockPos p_186185_3_, List<StructureComponent> p_186185_4_, Random p_186185_5_)
        {
            if (p_186185_1_ > 8)
            {
                return false;
            }
            else
            {
                Rotation rotation = p_186185_2_.field_186177_b.func_186215_c();
                StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate;
                p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(p_186185_2_, p_186185_3_, "base_floor", rotation, true));
                int i = p_186185_5_.nextInt(3);

                if (i == 0)
                {
                    p_186185_4_.add(StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-1, 4, -1), "base_roof", rotation, true));
                }
                else if (i == 1)
                {
                    StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate1;
                    p_186185_4_.add(structureendcitypieces$citytemplate1 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-1, 0, -1), "second_floor_2", rotation, false));
                    p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate1, new BlockPos(-1, 8, -1), "second_roof", rotation, false));
                    StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186206_f, p_186185_1_ + 1, structureendcitypieces$citytemplate, (BlockPos)null, p_186185_4_, p_186185_5_);
                }
                else if (i == 2)
                {
                    StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate2;
                    p_186185_4_.add(structureendcitypieces$citytemplate2 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-1, 0, -1), "second_floor_2", rotation, false));
                    p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate2, new BlockPos(-1, 4, -1), "third_floor_c", rotation, false));
                    StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate3;
                    p_186185_4_.add(structureendcitypieces$citytemplate3 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-1, 8, -1), "third_roof", rotation, true));
                    StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186206_f, p_186185_1_ + 1, structureendcitypieces$citytemplate3, (BlockPos)null, p_186185_4_, p_186185_5_);
                }

                return true;
            }
        }
    };
    private static final List<Tuple<Rotation, BlockPos>> field_186205_e = Lists.<Tuple<Rotation, BlockPos>>newArrayList(new Tuple[] {new Tuple(Rotation.NONE, new BlockPos(1, -1, 0)), new Tuple(Rotation.CLOCKWISE_90, new BlockPos(6, -1, 1)), new Tuple(Rotation.COUNTERCLOCKWISE_90, new BlockPos(0, -1, 5)), new Tuple(Rotation.CLOCKWISE_180, new BlockPos(5, -1, 6))});
    private static final StructureEndCityPieces.IGenerator field_186206_f = new StructureEndCityPieces.IGenerator()
    {
        public void func_186184_a()
        {
        }
        public boolean func_186185_a(int p_186185_1_, StructureEndCityPieces.CityTemplate p_186185_2_, BlockPos p_186185_3_, List<StructureComponent> p_186185_4_, Random p_186185_5_)
        {
            Rotation rotation = p_186185_2_.field_186177_b.func_186215_c();
            StructureEndCityPieces.CityTemplate lvt_7_1_;
            p_186185_4_.add(lvt_7_1_ = StructureEndCityPieces.func_186189_b(p_186185_2_, new BlockPos(3 + p_186185_5_.nextInt(2), -3, 3 + p_186185_5_.nextInt(2)), "tower_base", rotation, true));
            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate3;
            p_186185_4_.add(structureendcitypieces$citytemplate3 = StructureEndCityPieces.func_186189_b(lvt_7_1_, new BlockPos(0, 7, 0), "tower_piece", rotation, true));
            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate1 = p_186185_5_.nextInt(3) == 0 ? structureendcitypieces$citytemplate3 : null;
            int i = 1 + p_186185_5_.nextInt(3);

            for (int j = 0; j < i; ++j)
            {
                p_186185_4_.add(structureendcitypieces$citytemplate3 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate3, new BlockPos(0, 4, 0), "tower_piece", rotation, true));

                if (j < i - 1 && p_186185_5_.nextBoolean())
                {
                    structureendcitypieces$citytemplate1 = structureendcitypieces$citytemplate3;
                }
            }

            if (structureendcitypieces$citytemplate1 != null)
            {
                for (Tuple<Rotation, BlockPos> tuple : StructureEndCityPieces.field_186205_e)
                {
                    if (p_186185_5_.nextBoolean())
                    {
                        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate2;
                        p_186185_4_.add(structureendcitypieces$citytemplate2 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate1, (BlockPos)tuple.getSecond(), "bridge_end", rotation.func_185830_a((Rotation)tuple.getFirst()), true));
                        StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186207_g, p_186185_1_ + 1, structureendcitypieces$citytemplate2, (BlockPos)null, p_186185_4_, p_186185_5_);
                    }
                }

                p_186185_4_.add(StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate3, new BlockPos(-1, 4, -1), "tower_top", rotation, true));
            }
            else
            {
                if (p_186185_1_ != 7)
                {
                    return StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186209_i, p_186185_1_ + 1, structureendcitypieces$citytemplate3, (BlockPos)null, p_186185_4_, p_186185_5_);
                }

                p_186185_4_.add(StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate3, new BlockPos(-1, 4, -1), "tower_top", rotation, true));
            }

            return true;
        }
    };
    private static final StructureEndCityPieces.IGenerator field_186207_g = new StructureEndCityPieces.IGenerator()
    {
        public boolean field_186186_a;
        public void func_186184_a()
        {
            this.field_186186_a = false;
        }
        public boolean func_186185_a(int p_186185_1_, StructureEndCityPieces.CityTemplate p_186185_2_, BlockPos p_186185_3_, List<StructureComponent> p_186185_4_, Random p_186185_5_)
        {
            Rotation rotation = p_186185_2_.field_186177_b.func_186215_c();
            int i = p_186185_5_.nextInt(4) + 1;
            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate;
            p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(p_186185_2_, new BlockPos(0, 0, -4), "bridge_piece", rotation, true));
            structureendcitypieces$citytemplate.componentType = -1;
            int j = 0;

            for (int k = 0; k < i; ++k)
            {
                if (p_186185_5_.nextBoolean())
                {
                    p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(0, j, -4), "bridge_piece", rotation, true));
                    j = 0;
                }
                else
                {
                    if (p_186185_5_.nextBoolean())
                    {
                        p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(0, j, -4), "bridge_steep_stairs", rotation, true));
                    }
                    else
                    {
                        p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(0, j, -8), "bridge_gentle_stairs", rotation, true));
                    }

                    j = 4;
                }
            }

            if (!this.field_186186_a && p_186185_5_.nextInt(10 - p_186185_1_) == 0)
            {
                p_186185_4_.add(StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-8 + p_186185_5_.nextInt(8), j, -70 + p_186185_5_.nextInt(10)), "ship", rotation, true));
                this.field_186186_a = true;
            }
            else if (!StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186204_d, p_186185_1_ + 1, structureendcitypieces$citytemplate, new BlockPos(-3, j + 1, -11), p_186185_4_, p_186185_5_))
            {
                return false;
            }

            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate1;
            p_186185_4_.add(structureendcitypieces$citytemplate1 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(4, j, 0), "bridge_end", rotation.func_185830_a(Rotation.CLOCKWISE_180), true));
            structureendcitypieces$citytemplate1.componentType = -1;
            return true;
        }
    };
    private static final List<Tuple<Rotation, BlockPos>> field_186208_h = Lists.<Tuple<Rotation, BlockPos>>newArrayList(new Tuple[] {new Tuple(Rotation.NONE, new BlockPos(4, -1, 0)), new Tuple(Rotation.CLOCKWISE_90, new BlockPos(12, -1, 4)), new Tuple(Rotation.COUNTERCLOCKWISE_90, new BlockPos(0, -1, 8)), new Tuple(Rotation.CLOCKWISE_180, new BlockPos(8, -1, 12))});
    private static final StructureEndCityPieces.IGenerator field_186209_i = new StructureEndCityPieces.IGenerator()
    {
        public void func_186184_a()
        {
        }
        public boolean func_186185_a(int p_186185_1_, StructureEndCityPieces.CityTemplate p_186185_2_, BlockPos p_186185_3_, List<StructureComponent> p_186185_4_, Random p_186185_5_)
        {
            Rotation rotation = p_186185_2_.field_186177_b.func_186215_c();
            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate;
            p_186185_4_.add(structureendcitypieces$citytemplate = StructureEndCityPieces.func_186189_b(p_186185_2_, new BlockPos(-3, 4, -3), "fat_tower_base", rotation, true));
            StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate2;
            p_186185_4_.add(structureendcitypieces$citytemplate2 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate, new BlockPos(0, 4, 0), "fat_tower_middle", rotation, true));

            for (int i = 0; i < 2 && p_186185_5_.nextInt(3) != 0; ++i)
            {
                p_186185_4_.add(structureendcitypieces$citytemplate2 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate2, new BlockPos(0, 8, 0), "fat_tower_middle", rotation, true));

                for (Tuple<Rotation, BlockPos> tuple : StructureEndCityPieces.field_186208_h)
                {
                    if (p_186185_5_.nextBoolean())
                    {
                        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate1;
                        p_186185_4_.add(structureendcitypieces$citytemplate1 = StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate2, (BlockPos)tuple.getSecond(), "bridge_end", rotation.func_185830_a((Rotation)tuple.getFirst()), true));
                        StructureEndCityPieces.func_186187_b(StructureEndCityPieces.field_186207_g, p_186185_1_ + 1, structureendcitypieces$citytemplate1, (BlockPos)null, p_186185_4_, p_186185_5_);
                    }
                }
            }

            p_186185_4_.add(StructureEndCityPieces.func_186189_b(structureendcitypieces$citytemplate2, new BlockPos(-2, 8, -2), "fat_tower_top", rotation, true));
            return true;
        }
    };

    public static void registerPieces()
    {
        MapGenStructureIO.registerStructureComponent(StructureEndCityPieces.CityTemplate.class, "ECP");
    }

    private static StructureEndCityPieces.CityTemplate func_186189_b(StructureEndCityPieces.CityTemplate p_186189_0_, BlockPos p_186189_1_, String p_186189_2_, Rotation p_186189_3_, boolean p_186189_4_)
    {
        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate = new StructureEndCityPieces.CityTemplate(p_186189_2_, p_186189_0_.field_186178_c, p_186189_3_, p_186189_4_);
        BlockPos blockpos = p_186189_0_.field_186176_a.func_186262_a(p_186189_0_.field_186177_b, p_186189_1_, structureendcitypieces$citytemplate.field_186177_b, BlockPos.ORIGIN);
        structureendcitypieces$citytemplate.func_181138_a(blockpos.getX(), blockpos.getY(), blockpos.getZ());
        return structureendcitypieces$citytemplate;
    }

    public static void func_186190_a(BlockPos p_186190_0_, Rotation p_186190_1_, List<StructureComponent> p_186190_2_, Random p_186190_3_)
    {
        field_186209_i.func_186184_a();
        field_186204_d.func_186184_a();
        field_186207_g.func_186184_a();
        field_186206_f.func_186184_a();
        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate = new StructureEndCityPieces.CityTemplate("base_floor", p_186190_0_, p_186190_1_, true);
        p_186190_2_.add(structureendcitypieces$citytemplate);
        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate1;
        p_186190_2_.add(structureendcitypieces$citytemplate1 = func_186189_b(structureendcitypieces$citytemplate, new BlockPos(-1, 0, -1), "second_floor", p_186190_1_, false));
        StructureEndCityPieces.CityTemplate structureendcitypieces$citytemplate2;
        p_186190_2_.add(structureendcitypieces$citytemplate2 = func_186189_b(structureendcitypieces$citytemplate1, new BlockPos(-1, 4, -1), "third_floor", p_186190_1_, false));
        p_186190_2_.add(structureendcitypieces$citytemplate1 = func_186189_b(structureendcitypieces$citytemplate2, new BlockPos(-1, 8, -1), "third_roof", p_186190_1_, true));
        func_186187_b(field_186206_f, 1, structureendcitypieces$citytemplate1, (BlockPos)null, p_186190_2_, p_186190_3_);
    }

    private static boolean func_186187_b(StructureEndCityPieces.IGenerator p_186187_0_, int p_186187_1_, StructureEndCityPieces.CityTemplate p_186187_2_, BlockPos p_186187_3_, List<StructureComponent> p_186187_4_, Random p_186187_5_)
    {
        if (p_186187_1_ > 8)
        {
            return false;
        }
        else
        {
            List<StructureComponent> list = Lists.<StructureComponent>newArrayList();

            if (p_186187_0_.func_186185_a(p_186187_1_, p_186187_2_, p_186187_3_, list, p_186187_5_))
            {
                boolean flag = false;
                int i = p_186187_5_.nextInt();

                for (StructureComponent structurecomponent : list)
                {
                    structurecomponent.componentType = i;
                    StructureComponent structurecomponent1 = StructureComponent.findIntersecting(p_186187_4_, structurecomponent.getBoundingBox());

                    if (structurecomponent1 != null && structurecomponent1.componentType != p_186187_2_.componentType)
                    {
                        flag = true;
                        break;
                    }
                }

                if (!flag)
                {
                    p_186187_4_.addAll(list);
                    return true;
                }
            }

            return false;
        }
    }

    public static class CityTemplate extends StructureComponentTemplate
    {
        private String field_186181_d;
        private Rotation field_186182_e;
        private boolean field_186183_f;

        public CityTemplate()
        {
        }

        public CityTemplate(String p_i46634_1_, BlockPos p_i46634_2_, Rotation p_i46634_3_, boolean p_i46634_4_)
        {
            super(0);
            this.field_186181_d = p_i46634_1_;
            this.field_186182_e = p_i46634_3_;
            this.field_186183_f = p_i46634_4_;
            this.func_186180_a(p_i46634_2_);
        }

        private void func_186180_a(BlockPos p_186180_1_)
        {
            Template template = StructureEndCityPieces.field_186201_a.getTemplate((MinecraftServer)null, new ResourceLocation("endcity/" + this.field_186181_d));
            PlacementSettings placementsettings;

            if (this.field_186183_f)
            {
                placementsettings = StructureEndCityPieces.field_186202_b.func_186217_a().func_186220_a(this.field_186182_e);
            }
            else
            {
                placementsettings = StructureEndCityPieces.field_186203_c.func_186217_a().func_186220_a(this.field_186182_e);
            }

            this.func_186173_a(template, p_186180_1_, placementsettings);
        }

        protected void writeStructureToNBT(NBTTagCompound tagCompound)
        {
            super.writeStructureToNBT(tagCompound);
            tagCompound.setString("Template", this.field_186181_d);
            tagCompound.setString("Rot", this.field_186182_e.name());
            tagCompound.setBoolean("OW", this.field_186183_f);
        }

        protected void readStructureFromNBT(NBTTagCompound tagCompound)
        {
            super.readStructureFromNBT(tagCompound);
            this.field_186181_d = tagCompound.getString("Template");
            this.field_186182_e = Rotation.valueOf(tagCompound.getString("Rot"));
            this.field_186183_f = tagCompound.getBoolean("OW");
            this.func_186180_a(this.field_186178_c);
        }

        protected void func_186175_a(String p_186175_1_, BlockPos p_186175_2_, World p_186175_3_, Random p_186175_4_, StructureBoundingBox p_186175_5_)
        {
            if (p_186175_1_.startsWith("Chest"))
            {
                BlockPos blockpos = p_186175_2_.down();

                if (p_186175_5_.isVecInside(blockpos))
                {
                    TileEntity tileentity = p_186175_3_.getTileEntity(blockpos);

                    if (tileentity instanceof TileEntityChest)
                    {
                        ((TileEntityChest)tileentity).func_184287_a(LootTableList.CHESTS_END_CITY_TREASURE, p_186175_4_.nextLong());
                    }
                }
            }
            else if (p_186175_1_.startsWith("Sentry"))
            {
                EntityShulker entityshulker = new EntityShulker(p_186175_3_);
                entityshulker.setPosition((double)p_186175_2_.getX() + 0.5D, (double)p_186175_2_.getY() + 0.5D, (double)p_186175_2_.getZ() + 0.5D);
                entityshulker.func_184694_g(p_186175_2_);
                p_186175_3_.spawnEntityInWorld(entityshulker);
            }
            else if (p_186175_1_.startsWith("Elytra"))
            {
                EntityItemFrame entityitemframe = new EntityItemFrame(p_186175_3_, p_186175_2_, this.field_186182_e.func_185831_a(EnumFacing.SOUTH));
                entityitemframe.setDisplayedItem(new ItemStack(Items.elytra));
                p_186175_3_.spawnEntityInWorld(entityitemframe);
            }
        }
    }

    interface IGenerator
    {
        void func_186184_a();

        boolean func_186185_a(int p_186185_1_, StructureEndCityPieces.CityTemplate p_186185_2_, BlockPos p_186185_3_, List<StructureComponent> p_186185_4_, Random p_186185_5_);
    }
}
