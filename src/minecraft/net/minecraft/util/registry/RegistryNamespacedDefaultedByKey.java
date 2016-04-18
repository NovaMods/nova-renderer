package net.minecraft.util.registry;

import java.util.Random;
import org.apache.commons.lang3.Validate;

public class RegistryNamespacedDefaultedByKey<K, V> extends RegistryNamespaced<K, V>
{
    /** The key of the default value. */
    private final K defaultValueKey;

    /**
     * The default value for this registry, retrurned in the place of a null value.
     */
    private V defaultValue;

    public RegistryNamespacedDefaultedByKey(K defaultValueKeyIn)
    {
        this.defaultValueKey = defaultValueKeyIn;
    }

    public void register(int id, K key, V value)
    {
        if (this.defaultValueKey.equals(key))
        {
            this.defaultValue = value;
        }

        super.register(id, key, value);
    }

    /**
     * validates that this registry's key is non-null
     */
    public void validateKey()
    {
        Validate.notNull(this.defaultValue, "Missing default of DefaultedMappedRegistry: " + this.defaultValueKey, new Object[0]);
    }

    /**
     * Gets the integer ID we use to identify the given object.
     */
    public int getIDForObject(V value)
    {
        int i = super.getIDForObject(value);
        return i == -1 ? super.getIDForObject(this.defaultValue) : i;
    }

    /**
     * Gets the name we use to identify the given object.
     */
    public K getNameForObject(V value)
    {
        K k = super.getNameForObject(value);
        return (K)(k == null ? this.defaultValueKey : k);
    }

    public V getObject(K name)
    {
        V v = super.getObject(name);
        return (V)(v == null ? this.defaultValue : v);
    }

    /**
     * Gets the object identified by the given ID.
     */
    public V getObjectById(int id)
    {
        V v = super.getObjectById(id);
        return (V)(v == null ? this.defaultValue : v);
    }

    public V func_186801_a(Random random)
    {
        V v = super.func_186801_a(random);
        return (V)(v == null ? this.defaultValue : v);
    }
}
