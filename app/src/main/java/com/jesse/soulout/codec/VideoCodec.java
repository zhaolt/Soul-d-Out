package com.jesse.soulout.codec;

import android.animation.TimeAnimator;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMetadataRetriever;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import com.jesse.soulout.opengl.ISurface;
import com.jesse.soulout.util.VideoUtils;

import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public class VideoCodec implements VideoDecoderWrapper.OutputSampleListener {

    private static final String TAG = VideoCodec.class.getSimpleName();

    private ISurface mDisplay;

    private MediaExtractor mMediaExtractor;

    private VideoDecoderWrapper mVideoDecoderWrapper;

    private TimeAnimator mTimeAnimator = new TimeAnimator();

    private int mVideoWidth, mVideoHeight;

    private boolean isSupportHWDecode;

    private String mFilePath;


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
        mFilePath = filePath;
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        retriever.setDataSource(filePath);
        mVideoWidth = Integer.valueOf(retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH));
        mVideoHeight = Integer.valueOf(retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT));
        retriever.release();
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
                int fps = 0;
                fps = mediaFormat.getInteger(MediaFormat.KEY_FRAME_RATE);
                mDisplay.setVideoParameters(mVideoWidth, mVideoHeight, fps == 0 ? 30 : fps);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    isSupportHWDecode = VideoUtils.findHWDecoderByType2(mimeType);
                } else {
                    isSupportHWDecode = VideoUtils.findHWDecoderByType(mimeType);
                }
                if (isSupportHWDecode) {
                    mVideoDecoderWrapper.fromVideoFormat(mediaFormat, null);
                    mVideoDecoderWrapper.setOutputSampleListener(this);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void start() {
        if (isSupportHWDecode && null != mVideoDecoderWrapper) {
            mTimeAnimator.setTimeListener(new TimeAnimator.TimeListener() {
                @Override
                public void onTimeUpdate(TimeAnimator animation, long totalTime, long deltaTime) {
                    Log.e(TAG, "animation->totalTime: " + totalTime + ", deltaTime: " + deltaTime);
                    boolean isEos = ((mMediaExtractor.getSampleFlags() & MediaCodec
                            .BUFFER_FLAG_END_OF_STREAM) == MediaCodec.BUFFER_FLAG_END_OF_STREAM);


                    if (!isEos) {
                        boolean result = mVideoDecoderWrapper.writeSample(mMediaExtractor, false,
                                mMediaExtractor.getSampleTime(), mMediaExtractor.getSampleFlags());

                        if (result) {
                            mMediaExtractor.advance();
                        }
                    }


                    MediaCodec.BufferInfo out_bufferInfo = new MediaCodec.BufferInfo();
                    mVideoDecoderWrapper.peekSample(out_bufferInfo);


                    if (out_bufferInfo.size <= 0 && isEos) {
                        mTimeAnimator.end();
                        mVideoDecoderWrapper.stopAndRelease();
                        mMediaExtractor.release();
                    } else if (out_bufferInfo.presentationTimeUs / 1000 < totalTime) {
                        mVideoDecoderWrapper.popSample(false);
                    }
                }
            });
            mTimeAnimator.start();
        } else {
            decodeVideo(mFilePath);
        }
    }


    public void sendData2Java(byte[] data) {
        Log.e(TAG, "sendData2Java");
        mDisplay.offer(data);
    }


    private native int decodeVideo(String url);

    @Override
    public void outputSample(VideoDecoderWrapper sender, MediaCodec.BufferInfo info, ByteBuffer buffer) {
        byte[] data = new byte[info.size];
        buffer.get(data);
        mDisplay.offer(data);
    }
}
