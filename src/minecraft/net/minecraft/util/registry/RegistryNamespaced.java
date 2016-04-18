package net.minecraft.util.registry;

import com.google.common.collect.BiMap;
import com.google.common.collect.HashBiMap;
import java.util.Iterator;
import java.util.Map;
import net.minecraft.util.IObjectIntIterable;
import net.minecraft.util.IntIdentityHashBiMap;

public class RegistryNamespaced<K, V> extends RegistrySimple<K, V> implements IObjectIntIterable<V>
{
    protected final IntIdentityHashBiMap<V> underlyingIntegerMap = new IntIdentityHashBiMap(256);
    protected final Map<V, K> inverseObjectRegistry;

    public RegistryNamespaced()
    {
        this.inverseObjectRegistry = ((BiMap)this.registryObjects).inverse();
    }

    public void register(int id, K key, V value)
    {
        this.underlyingIntegerMap.func_186814_a(value, id);
        this.putObject(key, value);
    }

    protected Map<K, V> createUnderlyingMap()
    {
        return HashBiMap.<K, V>create();
    }

    public V getObject(K name)
    {
        return super.getObject(name);
    }

    /**
     * Gets the name we use to identify the given object.
     */
    public K getNameForObject(V value)
    {
        return (K)this.inverseObjectRegistry.get(value);
    }

    /**
     * Does this registry contain an entry for the given key?
     */
    public boolean containsKey(K key)
    {
        return super.containsKey(key);
    }

    /**
     * Gets the integer ID we use to identify the given object.
     */
    public int getIDForObject(V value)
    {
        return this.underlyingIntegerMap.func_186815_a(value);
    }

    /**
     * Gets the object identified by the given ID.
     */
    public V getObjectById(int id)
    {
        return (V)this.underlyingIntegerMap.func_186813_a(id);
    }

    public Iterator<V> iterator()
    {
        return this.underlyingIntegerMap.iterator();
    }
}
