package com.continuum.nova.utils;

import com.continuum.nova.NovaNative;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiScreen;
import net.minecraft.client.renderer.RegionRenderCache;
import net.minecraft.entity.Entity;
import net.minecraft.entity.EntityLivingBase;
import net.minecraft.init.MobEffects;
import net.minecraft.util.math.BlockPos;
import net.minecraft.world.IBlockAccess;
import net.minecraft.world.World;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.ArrayList;
import java.util.List;

/**
 * Creates the command to render a frame
 *
 * <p>Uses a ton f static methods because it doesn't have any internal state. I mostly created it so that render
 * command generation code doesn't clog up any other files.</p>
 *
 * @author David
 */
public class RenderCommandBuilder {
    private static final Logger LOG = LogManager.getLogger(RenderCommandBuilder.class);

    public static NovaNative.mc_render_command makeRenderCommand(Minecraft mc, float partialTicks) {
        NovaNative.mc_render_command command = new NovaNative.mc_render_command();

        addRenderWorldCommand(mc, partialTicks, command);

        return command;
    }

    public static NovaNative.mc_set_gui_screen_command createSetGuiScreenCommand(GuiScreen curScreen) {
        // Get all the buttons from the current GUI screen
        // TODO: Menu backgrounds, and everything else
        NovaNative.mc_set_gui_screen_command set_gui_screen_command = new NovaNative.mc_set_gui_screen_command();

        int cur_button = 0;
        for(GuiButton button : curScreen.getButtonList()) {
            NovaNative.mc_gui_button new_button = new NovaNative.mc_gui_button();
            new_button.x_position = button.xPosition;
            new_button.y_position = button.yPosition;
            new_button.width = button.getButtonWidth();
            new_button.height = button.getButtonHeight();
            new_button.text = button.displayString;

            set_gui_screen_command.screen.buttons[cur_button] = new_button;
        }

        return set_gui_screen_command;
    }

    private static void addRenderWorldCommand(Minecraft mc, double partialTicks, NovaNative.mc_render_command command) {
        Entity viewEntity = mc.getRenderViewEntity();

        if(Utils.exists(viewEntity)) {
            command.world_params.camera_x = viewEntity.lastTickPosX + (viewEntity.posX - viewEntity.lastTickPosX) * partialTicks;
            command.world_params.camera_y = viewEntity.lastTickPosY + (viewEntity.posY - viewEntity.lastTickPosY) * partialTicks;
            command.world_params.camera_z = viewEntity.lastTickPosZ + (viewEntity.posZ - viewEntity.lastTickPosZ) * partialTicks;
        }

        // TODO: Portal effects
    }

    private static NovaNative.mc_chunk makeChunk(World world, BlockPos chunkCoordinates) {
        NovaNative.mc_chunk chunk = new NovaNative.mc_chunk();
        NovaNative.mc_block[] blocks = new NovaNative.mc_block[16 * 16 * 16];

        // We really only want an array of block IDs
        IBlockAccess blockAccess = new RegionRenderCache(world, chunkCoordinates.add(-1, -1, -1), chunkCoordinates.add(16, 16, 16), 1);
        for(BlockPos.MutableBlockPos curPos : BlockPos.getAllInBoxMutable(chunkCoordinates, chunkCoordinates.add(15, 15, 15))) {
            IBlockState blockState = blockAccess.getBlockState(curPos);
            Block block = blockState.getBlock();
            int blockID = Block.getIdFromBlock(block);

            NovaNative.mc_block cur_block = new NovaNative.mc_block();
            cur_block.block_id = blockID;
            cur_block.is_on_fire = false;

            blocks[curPos.getX() + curPos.getY() * 16 + curPos.getZ() * 256] = cur_block;
        }

        chunk.blocks = (NovaNative.mc_block[]) blocks[0].toArray(blocks);

        return chunk;
    }

    private static List<NovaNative.mc_chunk> getAllChunks(World world, int renderChunkDistance) {
        List<NovaNative.mc_chunk> renderableChunks = new ArrayList<>();

        int chunkCountX = renderChunkDistance;
        int chunkCountY = 16;
        int chunkCountZ = renderChunkDistance;

        for(int x = 0; x < chunkCountX; x++) {
            for(int y = 0; y < chunkCountY; y++) {
                for(int z = 0; z < chunkCountZ; z++) {
                    BlockPos pos = new BlockPos(x * 16, y * 16, z * 16);
                    renderableChunks.add(makeChunk(world, pos));
                }
            }
        }

        return renderableChunks;
    }
}
