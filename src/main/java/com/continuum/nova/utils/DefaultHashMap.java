package com.continuum.nova.utils;

import java.util.HashMap;
import java.util.function.Supplier;

/**
 * @author ddubois
 * @since 29-Jul-17
 */
public class DefaultHashMap<K, V> extends HashMap<K, V> {
    private Supplier<V> valueSupplier;

    public DefaultHashMap(Supplier<V> valueSupplier) {
        this.valueSupplier = valueSupplier;
    }

    @Override
    public V get(Object key) {
        if(!containsKey(key)) {
            put((K) key, valueSupplier.get());
        }

        return get(key);
    }
}
