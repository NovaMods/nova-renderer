package com.continuum.nova.utils;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.HashMap;
import java.util.Map;

/**
 * Provides a simpler profiling interface than Minecraft's profiler
 *
 * @author ddubois
 * @since 31-Aug-17
 */
public class Profiler {
    private static final Logger LOG = LogManager.getLogger(Profiler.class);

    public static final int NUM_SAMPLES = 120;

    public static class ProfilerData {
        long startTime;
        long[] durations = new long[NUM_SAMPLES];
        int curDuration = 0;
        boolean rolledOver = false;

        public void start() {
            startTime = System.nanoTime();
        }

        public void stop() {
            long endTime = System.nanoTime();
            durations[curDuration] = endTime - startTime;
            curDuration++;

            if(curDuration >= NUM_SAMPLES) {
                rolledOver = true;
                curDuration = 0;
            }
        }

        public long getAverageDuration() {
            int num = rolledOver ? NUM_SAMPLES : curDuration;

            long accum = 0;
            for(int i = 0; i < num; i++) {
                accum += durations[i];
            }

            return accum / num;
        }
    }

    private static Map<String, ProfilerData> profilerDataMap = new HashMap<>();

    public static void start(String name) {
        if(!profilerDataMap.containsKey(name)) {
            profilerDataMap.put(name, new ProfilerData());
        }

        profilerDataMap.get(name).start();
    }

    public static void end(String name) {
        profilerDataMap.get(name).stop();
    }

    private static int counter = 0;

    public static void logData() {
        if(counter >= 100) {
            for(Map.Entry<String, ProfilerData> entry : profilerDataMap.entrySet()) {
                LOG.debug("Section {} has taken an average of {}ms", entry.getKey(), (double) entry.getValue().getAverageDuration() / 1000000.0);
            }
            counter = 0;
        }
        counter++;
    }

    private Profiler() {}
}
