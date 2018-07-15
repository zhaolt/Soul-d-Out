package com.jesse.soulout.util;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Log;

/**
 * Created by zhaoliangtai on 2018/6/26.
 */

public class VideoUtils {

    private static final String TAG = VideoUtils.class.getSimpleName();
    /**
     * 根据mime type寻找MediaCodec里对应的解码器
     * @param mime
     * @return
     */

    public static boolean findHWDecoderByType(String mime) {
        int numsOfCodec = MediaCodecList.getCodecCount();
        for (int i = 0; i < numsOfCodec; i++) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);
            if (info.isEncoder()) {
                continue;
            }
            String[] types = info.getSupportedTypes();
            boolean found = false;
            for (int j = 0; j < types.length && !found; j++) {
                if (types[j].equals(mime)) {
                    String codecName = info.getName();
                    Log.d(TAG, "find decoder name : " + codecName);
                    int[] colorFormats = info.getCapabilitiesForType(mime).colorFormats;
                    for (int k = 0; k < colorFormats.length; k++) {
                        if (colorFormats[k] == MediaCodecInfo.CodecCapabilities
                                .COLOR_FormatYUV420Flexible) {
                            Log.d(TAG, "codec " + codecName + " is support color format yuv420p");
                            return true;
                        }
                    }
                    return true;
                }
            }
            if (!found) {
                continue;
            }
        }
        return false;
    }
    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public static boolean findHWDecoderByType2(String mime) {
        MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
        MediaCodecInfo[] codecInfos = mediaCodecList.getCodecInfos();
        for (int i = 0, len = codecInfos.length; i < len; i++) {
            // 如果是编码器 则跳过
            if (codecInfos[i].isEncoder()) {
                continue;
            }
            String[] types = codecInfos[i].getSupportedTypes();
            boolean found = false;
            for (int j = 0; j < types.length && !found; j++) {
                if (types[j].equals(mime)) {
                    codecInfos[i].getCapabilitiesForType(mime);
                    String codecName = codecInfos[i].getName();
                    Log.d(TAG, "find decoder name : " + codecName);
                    int[] colorFormats = codecInfos[i].getCapabilitiesForType(mime).colorFormats;
                    for (int k = 0; k < colorFormats.length; k++) {
                        if (colorFormats[k] == MediaCodecInfo.CodecCapabilities
                                .COLOR_FormatYUV420Flexible) {
                            Log.d(TAG, "codec " + codecName + " is support color format yuv420p");
                            return true;
                        }
                    }
                }
            }
            if (!found) {
                continue;
            }
        }
        return false;
    }
}
