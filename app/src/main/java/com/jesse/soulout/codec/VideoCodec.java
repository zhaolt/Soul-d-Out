package com.jesse.soulout.codec;

import android.animation.TimeAnimator;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.text.TextUtils;

import com.jesse.soulout.opengl.ISurface;

import java.io.IOException;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public class VideoCodec {

    private static final String TAG = VideoCodec.class.getSimpleName();

    private ISurface mDisplay;

    private String mDataSource;

    private MediaExtractor mMediaExtractor;

    private MediaCodec mMediaCodec;

    private TimeAnimator mTimeAnimator;

    public VideoCodec() {
        mTimeAnimator = new TimeAnimator();
        mMediaExtractor = new MediaExtractor();
    }

    public void setDisplay(ISurface display) {
        mDisplay = display;
    }

    public void setDataSource(String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            throw new IllegalArgumentException("file path is NULL");
        }
        try {
            mMediaExtractor.setDataSource(filePath);
            int nTracks = mMediaExtractor.getTrackCount();
            for (int i = 0; i < nTracks; i++) {
                mMediaExtractor.unselectTrack(i);
            }
            MediaFormat mediaFormat = null;
            String mimeType = null;
            for (int i = 0; i < nTracks; i++) {
                mediaFormat = mMediaExtractor.getTrackFormat(i);
                mimeType = mediaFormat.getString(MediaFormat.KEY_MIME);
                if (mimeType.contains("video/")) {
                    mMediaExtractor.selectTrack(i);
                    break;
                }
            }
            if (null != mediaFormat && !TextUtils.isEmpty(mimeType)) {
                mMediaCodec = MediaCodec.createDecoderByType(mimeType);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void prepare() {

    }

    public void start() {

    }

    /**
     * 需要判断手机是否支持YUV420P
     */
}
