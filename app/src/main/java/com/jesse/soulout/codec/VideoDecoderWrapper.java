package com.jesse.soulout.codec;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Looper;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayDeque;
import java.util.Queue;

/**
 * Created by zhaoliangtai on 2018/7/15.
 */

public class VideoDecoderWrapper {
    private Handler mHandler;


    public interface OutputFormatChangedListener {
        void outputFormatChanged(VideoDecoderWrapper sender, MediaFormat newFormat);
    }

    private OutputFormatChangedListener mOutputFormatChangedListener = null;

    public interface OutputSampleListener {
        void outputSample(VideoDecoderWrapper sender, MediaCodec.BufferInfo info, ByteBuffer buffer);
    }

    private MediaCodec mDecoder;

    private ByteBuffer[] mInputBuffers;
    private ByteBuffer[] mOutputBuffers;

    private Queue<Integer> mAvailableInputBuffers;

    private Queue<Integer> mAvailableOutputBuffers;

    private MediaCodec.BufferInfo[] mOutputBufferInfo;

    private OutputSampleListener mOutputSampleListener;

    private VideoDecoderWrapper(MediaCodec codec) {
        mDecoder = codec;
        codec.start();
        mInputBuffers = codec.getInputBuffers();
        mOutputBuffers = codec.getOutputBuffers();
        mOutputBufferInfo = new MediaCodec.BufferInfo[mOutputBuffers.length];
        mAvailableInputBuffers = new ArrayDeque<>(mOutputBuffers.length);
        mAvailableOutputBuffers = new ArrayDeque<>(mInputBuffers.length);
    }

    public void stopAndRelease() {
        mDecoder.stop();
        mDecoder.release();
        mDecoder = null;
        mHandler = null;
    }

    public OutputFormatChangedListener getOutputFormatChangedListener() {
        return mOutputFormatChangedListener;
    }

    public void setOutputFormatChangedListener(final OutputFormatChangedListener
                                                       outputFormatChangedListener, Handler handler) {
        mOutputFormatChangedListener = outputFormatChangedListener;

        Looper looper;
        mHandler = handler;
        if (outputFormatChangedListener != null && mHandler == null) {
            if ((looper = Looper.myLooper()) != null) {
                mHandler = new Handler();
            } else {
                throw new IllegalArgumentException(
                        "Looper doesn't exist in the calling thread");
            }
        }
    }

    public static VideoDecoderWrapper fromVideoFormat(final MediaFormat trackFormat,
                                                    Surface surface) throws IOException {
        VideoDecoderWrapper result = null;
        MediaCodec videoCodec = null;


        final String mimeType = trackFormat.getString(MediaFormat.KEY_MIME);

        if (mimeType.contains("video/")) {
            videoCodec = MediaCodec.createDecoderByType(mimeType);
            trackFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities
                    .COLOR_FormatYUV420Planar);
            videoCodec.configure(trackFormat, surface, null,  0);
        }

        if (videoCodec != null) {
            result = new VideoDecoderWrapper(videoCodec);
        }


        return result;
    }


    public boolean writeSample(final ByteBuffer input,
                               final MediaCodec.CryptoInfo crypto,
                               final long presentationTimeUs,
                               final int flags) throws MediaCodec.CryptoException, WriteException {
        boolean result = false;
        int size = input.remaining();

        // check if we have dequed input buffers available from the codec
        if (size > 0 &&  !mAvailableInputBuffers.isEmpty()) {
            int index = mAvailableInputBuffers.remove();
            ByteBuffer buffer = mInputBuffers[index];

            // we can't write our sample to a lesser capacity input buffer.
            if (size > buffer.capacity()) {
                throw new WriteException(String.format(
                        "Insufficient capacity in MediaCodec buffer: "
                                + "tried to write %d, buffer capacity is %d.",
                        input.remaining(),
                        buffer.capacity()));
            }

            buffer.clear();
            buffer.put(input);

            if (crypto == null) {
                mDecoder.queueInputBuffer(index, 0, size, presentationTimeUs, flags);
            } else {
                mDecoder.queueSecureInputBuffer(index, 0, crypto, presentationTimeUs, flags);
            }
            result = true;
        }
        return result;
    }

    static MediaCodec.CryptoInfo cryptoInfo= new MediaCodec.CryptoInfo();

    public boolean writeSample(final MediaExtractor extractor,
                               final boolean isSecure,
                               final long presentationTimeUs,
                               int flags) {
        boolean result = false;
        boolean isEos = false;

        if (!mAvailableInputBuffers.isEmpty()) {
            int index = mAvailableInputBuffers.remove();
            ByteBuffer buffer = mInputBuffers[index];
            int size = extractor.readSampleData(buffer, 0);
            if (size <= 0) {
                flags |= MediaCodec.BUFFER_FLAG_END_OF_STREAM;
            }

            if (!isSecure) {
                mDecoder.queueInputBuffer(index, 0, size, presentationTimeUs, flags);
            } else {
                extractor.getSampleCryptoInfo(cryptoInfo);
                mDecoder.queueSecureInputBuffer(index, 0, cryptoInfo, presentationTimeUs, flags);
            }

            result = true;
        }
        return result;
    }

    public boolean peekSample(MediaCodec.BufferInfo out_bufferInfo) {
        update();
        boolean result = false;
        if (!mAvailableOutputBuffers.isEmpty()) {
            int index = mAvailableOutputBuffers.peek();
            MediaCodec.BufferInfo info = mOutputBufferInfo[index];
            out_bufferInfo.set(
                    info.offset,
                    info.size,
                    info.presentationTimeUs,
                    info.flags);
            result = true;
        }
        return result;
    }

    public void setOutputSampleListener(OutputSampleListener listener) {
        mOutputSampleListener = listener;
    }

    public void popSample(boolean render) {
        update();
        if (!mAvailableOutputBuffers.isEmpty()) {
            int index = mAvailableOutputBuffers.remove();

            if (mOutputSampleListener != null) {
                ByteBuffer buffer = mOutputBuffers[index];
                MediaCodec.BufferInfo info = mOutputBufferInfo[index];
                mOutputSampleListener.outputSample(this, info, buffer);
            }

            mDecoder.releaseOutputBuffer(index, render);
        }
    }

    private void update() {

        int index;

        while ((index = mDecoder.dequeueInputBuffer(0)) != MediaCodec.INFO_TRY_AGAIN_LATER) {
            mAvailableInputBuffers.add(index);
        }


        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        while ((index = mDecoder.dequeueOutputBuffer(info, 0)) !=  MediaCodec.INFO_TRY_AGAIN_LATER) {
            switch (index) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    mOutputBuffers = mDecoder.getOutputBuffers();
                    mOutputBufferInfo = new MediaCodec.BufferInfo[mOutputBuffers.length];
                    mAvailableOutputBuffers.clear();
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    if (mOutputFormatChangedListener != null) {
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                mOutputFormatChangedListener
                                        .outputFormatChanged(VideoDecoderWrapper.this,
                                                mDecoder.getOutputFormat());

                            }
                        });
                    }
                    break;
                default:
                    if(index >= 0) {
                        mOutputBufferInfo[index] = info;
                        mAvailableOutputBuffers.add(index);
                    } else {
                        throw new IllegalStateException("Unknown status from dequeueOutputBuffer");
                    }
                    break;
            }

        }


    }

    private class WriteException extends Throwable {
        private WriteException(final String detailMessage) {
            super(detailMessage);
        }
    }
}
