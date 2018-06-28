package com.jesse.soulout.opengl;

/**
 * Created by zhaoliangtai on 2018/6/25.
 */

public interface ISurface {

    /**
     * 将视频原始数据存入队列中
     * @param data
     */
    void offer(byte[] data);

    /**
     * 从队列头取出一组数据
     * @return
     */
    byte[] poll();

    /**
     * 设置视频信息
     * @param width
     * @param height
     * @param fps
     */
    void setVideoParameters(int width, int height, int fps);

    void start();

    void stop();

    void tryStop();
}
