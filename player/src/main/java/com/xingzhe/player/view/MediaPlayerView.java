package com.xingzhe.player.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.xingzhe.player.view.render.MediaPlayerRenderer;

public class MediaPlayerView extends GLSurfaceView {
    private MediaPlayerRenderer mRenderer;

    public MediaPlayerView(Context context) {
        this(context, null);
    }

    public MediaPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mRenderer = new MediaPlayerRenderer(context);
        setEGLContextClientVersion(2);
        setRenderer(mRenderer);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public void updateYUV(int width, int height, byte[] yData, byte[] uData, byte[] vData) {
        if (mRenderer != null) {
            mRenderer.updateYUVData(width, height, yData, uData, vData);
            requestRender();
        }
    }
}
