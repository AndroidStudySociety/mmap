package com.jesson.mmap;

public class ShareMemUtils {
    static {
        // 加载动态库
        System.loadLibrary("native-lib");
    }

    public native int open(String path, int length);

    public native void close(int fd);

    public native int get(int fd);

    public native byte[] read(int fd, int address);

    public native int write(int fd, byte[] buffer, int size, int address);

    public native boolean flush();

    public native void setMode(int mode);

    public native int getMode();
}
