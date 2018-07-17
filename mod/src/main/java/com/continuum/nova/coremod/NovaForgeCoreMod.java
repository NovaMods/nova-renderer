package com.continuum.nova.coremod;

import net.minecraft.launchwrapper.Launch;
import net.minecraft.launchwrapper.LaunchClassLoader;
import net.minecraftforge.fml.relauncher.IFMLLoadingPlugin;
import org.spongepowered.asm.launch.MixinBootstrap;
import org.spongepowered.asm.mixin.Mixins;

import java.lang.reflect.Field;
import java.util.Map;
import java.util.Set;

public class NovaForgeCoreMod implements IFMLLoadingPlugin {

    public NovaForgeCoreMod() {
        try {
            Field field = LaunchClassLoader.class.getDeclaredField("classLoaderExceptions");
            field.setAccessible(true);
            Set<String> s = (Set<String>) field.get(Launch.classLoader);
            s.remove("org.lwjgl.");
        } catch (NoSuchFieldException | IllegalAccessException e) {
            e.printStackTrace();
        }
        MixinBootstrap.init();
        Mixins.addConfiguration("mixins.nova.json");
    }

    @Override
    public String[] getASMTransformerClass() {
        return new String[] {"com.continuum.nova.transformer.NovaClassTransformer"};
    }

    @Override
    public String getModContainerClass() {
        return NovaForgeModContainer.class.getName();
    }

    @Override
    public String getSetupClass() {
        return null;
    }

    @Override
    public void injectData(Map<String, Object> data) {

    }

    @Override
    public String getAccessTransformerClass() {
        return null;
    }
}
