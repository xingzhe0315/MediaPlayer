package com.xingzhe.player.util;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;

public class GLESUtil {
    public static int createProgram(String vertexCode, String fragmentCode){
        int vertexShader = createShader(GLES20.GL_VERTEX_SHADER, vertexCode);
        int fragmentShader = createShader(GLES20.GL_FRAGMENT_SHADER,fragmentCode);
        int program = GLES20.glCreateProgram();
        GLES20.glAttachShader(program,vertexShader);
        GLES20.glAttachShader(program,fragmentShader);
        GLES20.glLinkProgram(program);
        int[] status = new int[1];
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS,status,0);
        if (status[0] ==0){
            Log.e("GLESUtil","----program link failed!!!");
            GLES20.glDeleteProgram(program);
            return -1;
        }
        return program;
    }

    public static  boolean validateProgram(int programId) {
        GLES20.glValidateProgram(programId);

        int[] validateStatus = new int[1];
        GLES20.glGetProgramiv(programId, GLES20.GL_VALIDATE_STATUS, validateStatus, 0);
//        Log.e(
//                "GLESUtil",
//                "result of validate program : " + validateStatus[0] + "\n : " + GLES20.glGetProgramInfoLog(programId)
//        );
        return validateStatus[0] != 0;
    }

    private static int createShader(int type, String sourceCode){
        int vertexShader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(vertexShader,sourceCode);
        GLES20.glCompileShader(vertexShader);
        int[] status = new int[1];
        GLES20.glGetShaderiv(vertexShader, GLES20.GL_COMPILE_STATUS,status,0);
        if (status[0] == 0){
            Log.e("GLESUtil","---compile shader failed : " + type);
            return -1;
        }
        return vertexShader;
    }

    public static int createOESTexture(){
        int []  texture = new int[1];
        GLES20.glGenTextures(1,texture,0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,texture[0]);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,  GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,  GLES20.GL_TEXTURE_MAG_FILTER,  GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,  GLES20.GL_TEXTURE_WRAP_S,  GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,  GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,0);
        return texture[0];
    }
}
