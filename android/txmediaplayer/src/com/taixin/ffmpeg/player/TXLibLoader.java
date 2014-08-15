package com.taixin.ffmpeg.player;

public interface TXLibLoader {
    public void loadLibrary(String libName) throws UnsatisfiedLinkError,
            SecurityException;
}
