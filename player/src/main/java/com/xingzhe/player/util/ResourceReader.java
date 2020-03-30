package com.xingzhe.player.util;

import android.content.Context;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;

public class ResourceReader {
    public static String readFromResource(Context context, int resId){
        InputStream inputStream = context.getResources().openRawResource(resId);
        BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(inputStream));
        StringBuilder stringBuilder = new StringBuilder();
        try {
            String line;
            while ((line = bufferedReader.readLine())!=null){
                stringBuilder.append(line).append("\n");
            }
        }catch (Exception e){

        }
        return stringBuilder.toString();
    }
}
