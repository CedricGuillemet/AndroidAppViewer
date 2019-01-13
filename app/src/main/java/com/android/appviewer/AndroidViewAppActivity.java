/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.appviewer;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;


public class AndroidViewAppActivity extends Activity implements AndroidViewAppSurfaceView.Renderer {

    AndroidViewAppSurfaceView mView;
    private float prevMouseX, prevMouseY;

    // JNI interface
    static {
        System.loadLibrary("rendererJNI");
    }

    public static native void init(Surface surface);
    public static native void resize(int width, int height);
    public static native void setMouseDelta(float dx, float dy);
    public static native void step();
    
    // Activity life
    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new AndroidViewAppSurfaceView(getApplication(), this);
        setContentView(mView);
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    // Renderer override
    @Override public void onSurfaceCreated(SurfaceHolder surfaceHolder){
        init(surfaceHolder.getSurface());
    }
    @Override public void onSurfaceChanged(int width, int height){
        resize(width, height);
    }
    @Override public void onDrawFrame(){
        step();
    }
    @Override public boolean onTouchEvent(MotionEvent event){
        switch(event.getAction()){
            case MotionEvent.ACTION_DOWN:
                prevMouseX = event.getX();
                prevMouseY = event.getY();
                break;
            case MotionEvent.ACTION_MOVE:
                float mX = event.getX();
                float mY = event.getY();
                float dx = mX - prevMouseX;
                float dy = mY - prevMouseY;
                prevMouseX = mX;
                prevMouseY = mY;
                setMouseDelta(dx, dy);
                break;
        }
        return true;
    }
}
