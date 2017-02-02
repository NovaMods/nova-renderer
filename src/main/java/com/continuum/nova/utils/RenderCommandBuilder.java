package com.continuum.nova.utils;

import com.continuum.nova.NovaNative;
import net.minecraft.block.Block;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.Minecraft;
import net.minecraft.client.gui.GuiButton;
import net.minecraft.client.gui.GuiScreen;
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

public class RenderCommandBuilder {
    private static final Logger LOG = LogManager.getLogger(RenderCommandBuilder.class);

    private static NovaNative.mc_chunk makeChunk(World world, BlockPos chunkCoordinates) {
        NovaNative.mc_chunk chunk = new NovaNative.mc_chunk();
        NovaNative.mc_block[] blocks = new NovaNative.mc_block[16 * 16 * 16];
        return chunk;
    }

    private static List<NovaNative.mc_chunk> getAllChunks(World world, int renderChunkDistance) {
        List<NovaNative.mc_chunk> renderableChunks = new ArrayList<>();
        int chunkCountX = renderChunkDistance;
        int chunkCountY = 16;
        int chunkCountZ = renderChunkDistance;

        for (int x = 0; x < chunkCountX; x++) {
            for (int y = 0; y < chunkCountY; y++) {
                for (int z = 0; z < chunkCountZ; z++) {
                    BlockPos pos = new BlockPos(x * 16, y * 16, z * 16);
                    renderableChunks.add(makeChunk(world, pos));
                }
            }
        }

        return renderableChunks;
    }
}
