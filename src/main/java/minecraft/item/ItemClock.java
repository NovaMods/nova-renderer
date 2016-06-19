package net.minecraft.item;

import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.math.MathHelper;
import net.minecraft.world.World;

public class ItemClock extends Item
{
    public ItemClock()
    {
        this.addPropertyOverride(new ResourceLocation("time"), new IItemPropertyGetter()
        {
            double field_185088_a;
            double field_185089_b;
            long field_185090_c;
            public float apply(ItemStack stack, World worldIn, EntityLivingBase entityIn)
            {
                boolean flag = entityIn != null;
                Entity entity = (Entity)(flag ? entityIn : stack.getItemFrame());

                if (worldIn == null && entity != null)
                {
                    worldIn = entity.worldObj;
                }

                if (worldIn == null)
                {
                    return 0.0F;
                }
                else
                {
                    double d0;

                    if (worldIn.provider.isSurfaceWorld())
                    {
                        d0 = (double)worldIn.getCelestialAngle(1.0F);
                    }
                    else
                    {
                        d0 = Math.random();
                    }

                    d0 = this.func_185087_a(worldIn, d0);
                    return MathHelper.func_188207_b((float)d0, 1.0F);
                }
            }
            private double func_185087_a(World p_185087_1_, double p_185087_2_)
            {
                if (p_185087_1_.getTotalWorldTime() != this.field_185090_c)
                {
                    this.field_185090_c = p_185087_1_.getTotalWorldTime();
                    double d0 = p_185087_2_ - this.field_185088_a;

                    if (d0 < -0.5D)
                    {
                        ++d0;
                    }

                    this.field_185089_b += d0 * 0.1D;
                    this.field_185089_b *= 0.9D;
                    this.field_185088_a += this.field_185089_b;
                }

                return this.field_185088_a;
            }
        });
    }
}
