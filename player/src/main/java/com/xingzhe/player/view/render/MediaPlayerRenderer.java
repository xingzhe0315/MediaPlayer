package com.xingzhe.player.view.render;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.xingzhe.player.R;
import com.xingzhe.player.util.GLESUtil;
import com.xingzhe.player.util.ResourceReader;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MediaPlayerRenderer implements GLSurfaceView.Renderer {
    public static final String A_POSITION = "a_Position";
    public static final String A_TEXTURECOORDINATES = "a_TextureCoordinates";
    public static final String Y_SAMPLER = "y_Sampler";
    public static final String U_SAMPLER = "u_Sampler";
    public static final String V_SAMPLER = "v_Sampler";

    private final float[] vertexData = {//顶点坐标
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };

    private final float[] textureData = {//纹理坐标
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private Context mContext;

    private int glProgram;
    private int verPosLocation;
    private int texturePosLocation;
    private int ySampler;
    private int uSampler;
    private int vSampler;
    private int[] textureIdArray;

    private FloatBuffer mVertexBuffer;
    private FloatBuffer mTextureBuffer;

    private int width;
    private int height;
    private ByteBuffer yBuffer;
    private ByteBuffer uBuffer;
    private ByteBuffer vBuffer;

    public MediaPlayerRenderer(Context context) {
        mContext = context;
        mVertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mVertexBuffer.put(vertexData);
        mTextureBuffer = ByteBuffer.allocateDirect(textureData.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        mTextureBuffer.put(textureData);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        String vertexSource = ResourceReader.readFromResource(mContext, R.raw.texture_vertex_shader);
        String fragmentSource = ResourceReader.readFromResource(mContext, R.raw.texture_fragment_shader);
        glProgram = GLESUtil.createProgram(vertexSource, fragmentSource);
        verPosLocation = GLES20.glGetAttribLocation(glProgram, A_POSITION);
        texturePosLocation = GLES20.glGetAttribLocation(glProgram, A_TEXTURECOORDINATES);
        ySampler = GLES20.glGetUniformLocation(glProgram, Y_SAMPLER);
        uSampler = GLES20.glGetUniformLocation(glProgram, U_SAMPLER);
        vSampler = GLES20.glGetUniformLocation(glProgram, V_SAMPLER);

        mVertexBuffer.position(0);
        mTextureBuffer.position(0);


        //创建纹理
        textureIdArray = new int[3];
        GLES20.glGenTextures(3, textureIdArray, 0);

        for (int i = 0; i < 3; i++) {
            //绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureIdArray[i]);
            //设置环绕和过滤方式
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        if (width == 0 || height == 0 || yBuffer == null || uBuffer == null || vBuffer == null) return;

        GLES20.glUseProgram(glProgram);

        GLES20.glVertexAttribPointer(verPosLocation, 2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);
        GLES20.glVertexAttribPointer(texturePosLocation, 2, GLES20.GL_FLOAT, false, 8, mTextureBuffer);
        GLES20.glEnableVertexAttribArray(verPosLocation);
        GLES20.glEnableVertexAttribArray(texturePosLocation);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureIdArray[0]);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width, height, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, yBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureIdArray[1]);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width / 2, height / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, uBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureIdArray[2]);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width / 2, height / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, vBuffer);

        GLES20.glUniform1i(ySampler, 0);
        GLES20.glUniform1i(uSampler, 1);
        GLES20.glUniform1i(vSampler, 2);

        yBuffer.clear();
        uBuffer.clear();
        vBuffer.clear();

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    public void updateYUVData(int width, int height, byte[] yData, byte[] uData, byte[] vData) {
        this.width = width;
        this.height = height;

        yBuffer = ByteBuffer.wrap(yData);
        uBuffer = ByteBuffer.wrap(uData);
        vBuffer = ByteBuffer.wrap(vData);
    }
}
