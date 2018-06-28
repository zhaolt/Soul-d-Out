package com.jesse.soulout.util;

import android.content.Context;
import android.content.res.AssetManager;

import java.io.IOException;
import java.io.InputStream;

/**
 * Created by zhaoliangtai on 2018/6/26.
 */

public class AssetsUtils {


    public static String getAssetsContents(Context context, String path) {
        AssetManager assetManager = context.getAssets();
        InputStream is = null;
        try {
            is = assetManager.open(path, AssetManager.ACCESS_BUFFER);
            byte[] buffer = new byte[is.available()];
            is.read(buffer);
            return new String(buffer, "utf-8");
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (null != is) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return null;
    }
}
