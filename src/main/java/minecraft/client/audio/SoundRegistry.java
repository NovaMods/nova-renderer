package net.minecraft.client.audio;

import com.google.common.collect.Maps;
import java.util.Map;
import net.minecraft.util.ResourceLocation;
import net.minecraft.util.registry.RegistrySimple;

public class SoundRegistry extends RegistrySimple<ResourceLocation, SoundEventAccessor>
{
    private Map<ResourceLocation, SoundEventAccessor> soundRegistry;

    protected Map<ResourceLocation, SoundEventAccessor> createUnderlyingMap()
    {
        this.soundRegistry = Maps.<ResourceLocation, SoundEventAccessor>newHashMap();
        return this.soundRegistry;
    }

    public void func_186803_a(SoundEventAccessor p_186803_1_)
    {
        this.putObject(p_186803_1_.func_188714_b(), p_186803_1_);
    }

    /**
     * Reset the underlying sound map (Called on resource manager reload)
     */
    public void clearMap()
    {
        this.soundRegistry.clear();
    }
}
