package net.minecraft.util.registry;

import java.util.Set;

public interface IRegistry<K, V> extends Iterable<V>
{
    V getObject(K name);

    /**
     * Register an object on this registry.
     */
    void putObject(K key, V value);

    Set<K> getKeys();
}
