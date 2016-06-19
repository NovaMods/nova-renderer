package net.minecraft.client.renderer.block.model;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParseException;
import java.lang.reflect.Type;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.util.math.MathHelper;
import org.lwjgl.util.vector.Quaternion;

public class ItemCameraTransforms
{
    public static final ItemCameraTransforms DEFAULT = new ItemCameraTransforms();
    public static float field_181690_b = 0.0F;
    public static float field_181691_c = 0.0F;
    public static float field_181692_d = 0.0F;
    public static float field_181693_e = 0.0F;
    public static float field_181694_f = 0.0F;
    public static float field_181695_g = 0.0F;
    public static float field_181696_h = 0.0F;
    public static float field_181697_i = 0.0F;
    public static float field_181698_j = 0.0F;
    public final ItemTransformVec3f thirdperson_left;
    public final ItemTransformVec3f thirdperson_right;
    public final ItemTransformVec3f firstperson_left;
    public final ItemTransformVec3f firstperson_right;
    public final ItemTransformVec3f head;
    public final ItemTransformVec3f gui;
    public final ItemTransformVec3f ground;
    public final ItemTransformVec3f fixed;

    private ItemCameraTransforms()
    {
        this(ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT, ItemTransformVec3f.DEFAULT);
    }

    public ItemCameraTransforms(ItemCameraTransforms transforms)
    {
        this.thirdperson_left = transforms.thirdperson_left;
        this.thirdperson_right = transforms.thirdperson_right;
        this.firstperson_left = transforms.firstperson_left;
        this.firstperson_right = transforms.firstperson_right;
        this.head = transforms.head;
        this.gui = transforms.gui;
        this.ground = transforms.ground;
        this.fixed = transforms.fixed;
    }

    public ItemCameraTransforms(ItemTransformVec3f p_i46569_1_, ItemTransformVec3f p_i46569_2_, ItemTransformVec3f p_i46569_3_, ItemTransformVec3f p_i46569_4_, ItemTransformVec3f p_i46569_5_, ItemTransformVec3f p_i46569_6_, ItemTransformVec3f p_i46569_7_, ItemTransformVec3f p_i46569_8_)
    {
        this.thirdperson_left = p_i46569_1_;
        this.thirdperson_right = p_i46569_2_;
        this.firstperson_left = p_i46569_3_;
        this.firstperson_right = p_i46569_4_;
        this.head = p_i46569_5_;
        this.gui = p_i46569_6_;
        this.ground = p_i46569_7_;
        this.fixed = p_i46569_8_;
    }

    public void applyTransform(ItemCameraTransforms.TransformType type)
    {
        func_188034_a(this.getTransform(type), false);
    }

    public static void func_188034_a(ItemTransformVec3f p_188034_0_, boolean p_188034_1_)
    {
        if (p_188034_0_ != ItemTransformVec3f.DEFAULT)
        {
            int i = p_188034_1_ ? -1 : 1;
            GlStateManager.translate((float)i * (field_181690_b + p_188034_0_.translation.x), field_181691_c + p_188034_0_.translation.y, field_181692_d + p_188034_0_.translation.z);
            float f = field_181693_e + p_188034_0_.rotation.x;
            float f1 = field_181694_f + p_188034_0_.rotation.y;
            float f2 = field_181695_g + p_188034_0_.rotation.z;

            if (p_188034_1_)
            {
                f1 = -f1;
                f2 = -f2;
            }

            GlStateManager.rotate(func_188035_a(f, f1, f2));
            GlStateManager.scale(field_181696_h + p_188034_0_.scale.x, field_181697_i + p_188034_0_.scale.y, field_181698_j + p_188034_0_.scale.z);
        }
    }

    private static Quaternion func_188035_a(float p_188035_0_, float p_188035_1_, float p_188035_2_)
    {
        float f = p_188035_0_ * 0.017453292F;
        float f1 = p_188035_1_ * 0.017453292F;
        float f2 = p_188035_2_ * 0.017453292F;
        float f3 = MathHelper.sin(0.5F * f);
        float f4 = MathHelper.cos(0.5F * f);
        float f5 = MathHelper.sin(0.5F * f1);
        float f6 = MathHelper.cos(0.5F * f1);
        float f7 = MathHelper.sin(0.5F * f2);
        float f8 = MathHelper.cos(0.5F * f2);
        return new Quaternion(f3 * f6 * f8 + f4 * f5 * f7, f4 * f5 * f8 - f3 * f6 * f7, f3 * f5 * f8 + f4 * f6 * f7, f4 * f6 * f8 - f3 * f5 * f7);
    }

    public ItemTransformVec3f getTransform(ItemCameraTransforms.TransformType type)
    {
        switch (type)
        {
            case THIRD_PERSON_LEFT_HAND:
                return this.thirdperson_left;

            case THIRD_PERSON_RIGHT_HAND:
                return this.thirdperson_right;

            case FIRST_PERSON_LEFT_HAND:
                return this.firstperson_left;

            case FIRST_PERSON_RIGHT_HAND:
                return this.firstperson_right;

            case HEAD:
                return this.head;

            case GUI:
                return this.gui;

            case GROUND:
                return this.ground;

            case FIXED:
                return this.fixed;

            default:
                return ItemTransformVec3f.DEFAULT;
        }
    }

    public boolean func_181687_c(ItemCameraTransforms.TransformType type)
    {
        return this.getTransform(type) != ItemTransformVec3f.DEFAULT;
    }

    static class Deserializer implements JsonDeserializer<ItemCameraTransforms>
    {
        public ItemCameraTransforms deserialize(JsonElement p_deserialize_1_, Type p_deserialize_2_, JsonDeserializationContext p_deserialize_3_) throws JsonParseException
        {
            JsonObject jsonobject = p_deserialize_1_.getAsJsonObject();
            ItemTransformVec3f itemtransformvec3f = this.func_181683_a(p_deserialize_3_, jsonobject, "thirdperson_righthand");
            ItemTransformVec3f itemtransformvec3f1 = this.func_181683_a(p_deserialize_3_, jsonobject, "thirdperson_lefthand");

            if (itemtransformvec3f1 == ItemTransformVec3f.DEFAULT)
            {
                itemtransformvec3f1 = itemtransformvec3f;
            }

            ItemTransformVec3f itemtransformvec3f2 = this.func_181683_a(p_deserialize_3_, jsonobject, "firstperson_righthand");
            ItemTransformVec3f itemtransformvec3f3 = this.func_181683_a(p_deserialize_3_, jsonobject, "firstperson_lefthand");

            if (itemtransformvec3f3 == ItemTransformVec3f.DEFAULT)
            {
                itemtransformvec3f3 = itemtransformvec3f2;
            }

            ItemTransformVec3f itemtransformvec3f4 = this.func_181683_a(p_deserialize_3_, jsonobject, "head");
            ItemTransformVec3f itemtransformvec3f5 = this.func_181683_a(p_deserialize_3_, jsonobject, "gui");
            ItemTransformVec3f itemtransformvec3f6 = this.func_181683_a(p_deserialize_3_, jsonobject, "ground");
            ItemTransformVec3f itemtransformvec3f7 = this.func_181683_a(p_deserialize_3_, jsonobject, "fixed");
            return new ItemCameraTransforms(itemtransformvec3f1, itemtransformvec3f, itemtransformvec3f3, itemtransformvec3f2, itemtransformvec3f4, itemtransformvec3f5, itemtransformvec3f6, itemtransformvec3f7);
        }

        private ItemTransformVec3f func_181683_a(JsonDeserializationContext p_181683_1_, JsonObject p_181683_2_, String p_181683_3_)
        {
            return p_181683_2_.has(p_181683_3_) ? (ItemTransformVec3f)p_181683_1_.deserialize(p_181683_2_.get(p_181683_3_), ItemTransformVec3f.class) : ItemTransformVec3f.DEFAULT;
        }
    }

    public static enum TransformType
    {
        NONE,
        THIRD_PERSON_LEFT_HAND,
        THIRD_PERSON_RIGHT_HAND,
        FIRST_PERSON_LEFT_HAND,
        FIRST_PERSON_RIGHT_HAND,
        HEAD,
        GUI,
        GROUND,
        FIXED;
    }
}
