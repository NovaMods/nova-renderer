package com.continuum.nova.nativethread;

import java.time.LocalDateTime;

/**
 * An API to monitor the native rendering thread from
 *
 * <p>Includes functionality to start and stop the thread, wake the thread (maybe), and check if the thread if available
 * </p>
 *
 * @author David
 */
public class NativeThreadMonitor {
    private long nativeHandle;
    private LocalDateTime threadLastActiveTime;

    public NativeThreadMonitor() {
        initialize();
    }

    public native void startThread();

    public native void stopThread();

    public LocalDateTime getThreadLastActiveTime() {
        return threadLastActiveTime;
    }

    /**
     * Acts as a constructor for the native object
     */
    private native long initialize();
}
