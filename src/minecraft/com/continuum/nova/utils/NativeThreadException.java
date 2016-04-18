package com.continuum.nova.utils;

/**
 * Simple exception to handle problems with the native thread
 *
 * @author David
 */
public class NativeThreadException extends Exception {
    public NativeThreadException(String message) {
        super(message);
    }
}
