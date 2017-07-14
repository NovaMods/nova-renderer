package com.continuum.nova.chunks;

import com.google.gson.*;
import net.minecraft.block.state.IBlockState;
import net.minecraft.client.renderer.block.model.BakedQuad;
import net.minecraft.client.renderer.block.model.IBakedModel;
import net.minecraft.util.EnumFacing;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Provides a way to write all the block models to disk in a Nova-friendly format
 *
 * @author ddubois
 * @since 11-Jul-17
 */
public class BlockModelSerializer {
    private static final Logger LOG = LogManager.getLogger(BlockModelSerializer.class);

    /**
     * Maps from model to the geometry in the model
     *
     * <p>The geometry in the model is represented by a map from the facing direction of the face to the face itself</p>
     *
     * <p>The first string is the block state of the model. The second string is the name of the facing direction of
     * the face</p>
     */
    private Map<String, Map<String, List<BakedQuad>>> allModels = new HashMap<>();

    /**
     * Adds the provided model to the list of models
     *
     * @param state The block state that the given model represents
     * @param model The model to add
     */
    public void addModel(IBlockState state, IBakedModel model) {
        Map<String, List<BakedQuad>> modelData = new HashMap<>();

        for(EnumFacing face : EnumFacing.values()) {
            List<BakedQuad> quads = model.getQuads(state, face, 0);
            modelData.put(face.getName(), quads);
        }

        allModels.put(state.toString(), modelData);
        LOG.trace("Added a model for state {}", state);
    }

    /**
     * Clears the data in this BlockModelSerializer. Useful when you change resourcepacks or whatever
     */
    public void clear() {
        allModels = new HashMap<>();
    }

    /**
     * Saves the data in this class to the specified file
     *
     * @param filename The name of the file to save the block model data to
     */
    public void serialize(String filename) throws IOException {
        File file = new File(filename);
        file.mkdirs();

        Gson gson = new GsonBuilder()
                //.setPrettyPrinting()
                .registerTypeAdapter(BakedQuad.class, new BakedQuadAdapter())
                .create();

        // Write one JSON file for each model. I'd like to put them all in one big model but that isn't really working
        String jsonString = gson.toJson(this);
        try(BufferedWriter writer = new BufferedWriter(new FileWriter(filename))) {
            writer.append(jsonString);
        }

        LOG.debug("Wrote all models to folder {}", filename);
    }

    public static class BakedQuadAdapter implements JsonSerializer<BakedQuad> {
        @Override
        public JsonElement serialize(BakedQuad bakedQuad, Type type, JsonSerializationContext jsonSerializationContext) {
            JsonArray arr = new JsonArray();
            for(int i : bakedQuad.getVertexData()) {
                arr.add(new JsonPrimitive(i));
            }

            JsonObject obj = new JsonObject();
            obj.add("vertexData", arr);
            obj.addProperty("tintIndex", bakedQuad.getTintIndex());
            obj.addProperty("face", bakedQuad.getFace().toString());
            obj.addProperty("sprite", bakedQuad.getSprite().getIconName());

            return obj;
        }
    }
}
