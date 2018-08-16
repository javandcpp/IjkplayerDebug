package ijkj4a.java.java.nio;

import android.os.Build;

import java.nio.*;

@SimpleCClassName
@IncludeUtil
public class ByteBuffer {
    public static ByteBuffer allocate(int capacity);
    public static ByteBuffer allocateDirect(int capacity);
    public final java.nio.Buffer limit(int newLimit);
}
