package net.minecraft.client.renderer.tileentity;

import com.mojang.authlib.GameProfile;
import java.util.UUID;
import net.minecraft.block.Block;
import net.minecraft.block.BlockChest;
import net.minecraft.client.Minecraft;
import net.minecraft.client.model.ModelShield;
import net.minecraft.client.renderer.BannerTextures;
import net.minecraft.client.renderer.GlStateManager;
import net.minecraft.init.Blocks;
import net.minecraft.init.Items;
import net.minecraft.item.ItemStack;
import net.minecraft.nbt.NBTTagCompound;
import net.minecraft.nbt.NBTUtil;
import net.minecraft.tileentity.TileEntityBanner;
import net.minecraft.tileentity.TileEntityChest;
import net.minecraft.tileentity.TileEntityEnderChest;
import net.minecraft.tileentity.TileEntitySkull;
import net.minecraft.util.EnumFacing;

public class TileEntityItemStackRenderer
{
    public static TileEntityItemStackRenderer instance = new TileEntityItemStackRenderer();
    private TileEntityChest field_147717_b = new TileEntityChest(BlockChest.Type.BASIC);
    private TileEntityChest field_147718_c = new TileEntityChest(BlockChest.Type.TRAP);
    private TileEntityEnderChest enderChest = new TileEntityEnderChest();
    private TileEntityBanner banner = new TileEntityBanner();
    private TileEntitySkull skull = new TileEntitySkull();
    private ModelShield field_187318_g = new ModelShield();

    public void renderByItem(ItemStack itemStackIn)
    {
        if (itemStackIn.getItem() == Items.banner)
        {
            this.banner.setItemValues(itemStackIn);
            TileEntityRendererDispatcher.instance.renderTileEntityAt(this.banner, 0.0D, 0.0D, 0.0D, 0.0F);
        }
        else if (itemStackIn.getItem() == Items.shield)
        {
            if (itemStackIn.getSubCompound("BlockEntityTag", false) != null)
            {
                this.banner.setItemValues(itemStackIn);
                Minecraft.getMinecraft().getTextureManager().bindTexture(BannerTextures.field_187485_b.func_187478_a(this.banner.getPatternResourceLocation(), this.banner.getPatternList(), this.banner.getColorList()));
            }
            else
            {
                Minecraft.getMinecraft().getTextureManager().bindTexture(BannerTextures.field_187486_c);
            }

            GlStateManager.pushMatrix();
            GlStateManager.scale(1.0F, -1.0F, -1.0F);
            this.field_187318_g.func_187062_a();
            GlStateManager.popMatrix();
        }
        else if (itemStackIn.getItem() == Items.skull)
        {
            GameProfile gameprofile = null;

            if (itemStackIn.hasTagCompound())
            {
                NBTTagCompound nbttagcompound = itemStackIn.getTagCompound();

                if (nbttagcompound.hasKey("SkullOwner", 10))
                {
                    gameprofile = NBTUtil.readGameProfileFromNBT(nbttagcompound.getCompoundTag("SkullOwner"));
                }
                else if (nbttagcompound.hasKey("SkullOwner", 8) && !nbttagcompound.getString("SkullOwner").isEmpty())
                {
                    GameProfile lvt_2_2_ = new GameProfile((UUID)null, nbttagcompound.getString("SkullOwner"));
                    gameprofile = TileEntitySkull.updateGameprofile(lvt_2_2_);
                    nbttagcompound.removeTag("SkullOwner");
                    nbttagcompound.setTag("SkullOwner", NBTUtil.writeGameProfile(new NBTTagCompound(), gameprofile));
                }
            }

            if (TileEntitySkullRenderer.instance != null)
            {
                GlStateManager.pushMatrix();
                GlStateManager.disableCull();
                TileEntitySkullRenderer.instance.renderSkull(0.0F, 0.0F, 0.0F, EnumFacing.UP, 0.0F, itemStackIn.getMetadata(), gameprofile, -1, 0.0F);
                GlStateManager.enableCull();
                GlStateManager.popMatrix();
            }
        }
        else
        {
            Block block = Block.getBlockFromItem(itemStackIn.getItem());

            if (block == Blocks.ender_chest)
            {
                TileEntityRendererDispatcher.instance.renderTileEntityAt(this.enderChest, 0.0D, 0.0D, 0.0D, 0.0F);
            }
            else if (block == Blocks.trapped_chest)
            {
                TileEntityRendererDispatcher.instance.renderTileEntityAt(this.field_147718_c, 0.0D, 0.0D, 0.0D, 0.0F);
            }
            else
            {
                TileEntityRendererDispatcher.instance.renderTileEntityAt(this.field_147717_b, 0.0D, 0.0D, 0.0D, 0.0F);
            }
        }
    }
}
