package com.ywl5320.listener;

import android.graphics.Bitmap;
import android.view.Surface;

/**
 * Created by hlwky001 on 2017/12/18.
 */

public interface WlOnGlSurfaceViewOnCreateListener {

    void onGlSurfaceViewOnCreate(Surface surface);

    void onCutVideoImg(Bitmap bitmap);

}
