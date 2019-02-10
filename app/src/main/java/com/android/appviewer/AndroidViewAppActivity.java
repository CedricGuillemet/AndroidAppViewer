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
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.Toast;
import java.lang.Math;

public class AndroidViewAppActivity extends Activity implements AndroidViewAppSurfaceView.Renderer, SensorEventListener {

    AndroidViewAppSurfaceView mView;
    private float prevMouseX, prevMouseY;
    private SensorManager mSensorManager;
    private Sensor mRotationSensor;
    private static final int SENSOR_DELAY = 1000; // 500m

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

        try {
            mSensorManager = (SensorManager) getSystemService(Activity.SENSOR_SERVICE);
            mRotationSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
            mSensorManager.registerListener(this, mRotationSensor, SENSOR_DELAY);
        } catch (Exception e) {
            Toast.makeText(this, "Hardware compatibility issue", Toast.LENGTH_LONG).show();
        }
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        //mSensorManager.unregisterListener(this);
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        /*
        Sensor accelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        if (accelerometer != null) {
            mSensorManager.registerListener(this, accelerometer,
                    SensorManager.SENSOR_DELAY_NORMAL, SensorManager.SENSOR_DELAY_UI);
        }
        Sensor magneticField = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        if (magneticField != null) {
            mSensorManager.registerListener(this, magneticField,
                    SensorManager.SENSOR_DELAY_NORMAL, SensorManager.SENSOR_DELAY_UI);
        }
        */
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor == mRotationSensor) {
            if (event.values.length > 4) {
                float[] truncatedRotationVector = new float[4];
                System.arraycopy(event.values, 0, truncatedRotationVector, 0, 4);
                update(truncatedRotationVector);
            } else {
                update(event.values);
            }
        }
    }

    private void update(float[] vectors) {
        float[] rotationMatrix = new float[9];
        SensorManager.getRotationMatrixFromVector(rotationMatrix, vectors);
        int worldAxisX = SensorManager.AXIS_X;
        int worldAxisY = SensorManager.AXIS_Y;
        float[] adjustedRotationMatrix = new float[9];
        SensorManager.remapCoordinateSystem(rotationMatrix, worldAxisX, worldAxisY, adjustedRotationMatrix);
        float[] orientation = new float[3];
        SensorManager.getOrientation(adjustedRotationMatrix, orientation);
        //float pitch = orientation[1] * FROM_RADS_TO_DEGS;
        //float roll = orientation[2] * FROM_RADS_TO_DEGS;

        setMouseDelta((rotationMatrix[3]+rotationMatrix[4]+rotationMatrix[5])*2.f,
                -(rotationMatrix[0]+rotationMatrix[1]+rotationMatrix[2])*2.f);

        //Log.e("r", " "+(rotationMatrix[0]+rotationMatrix[3]+rotationMatrix[6]));
        //Log.e("r", rotationMatrix[0] + "/" );//+ rotationMatrix[1]+"/"+rotationMatrix[2]);
        //Log.e("r", rotationMatrix[3] + "/" + rotationMatrix[4]+"/"+rotationMatrix[5]);
        //Log.e("r", rotationMatrix[6] + "/" + rotationMatrix[7]+"/"+rotationMatrix[8]);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Do something here if sensor accuracy changes.
        // You must implement this callback in your code.
    }
    // Renderer override
    @Override public void onSurfaceCreated(SurfaceHolder surfaceHolder){
        init(surfaceHolder.getSurface());
    }
    @Override public void onSurfaceChanged(int width, int height){
        resize(width, height);
    }
    @Override public void onDrawFrame(){
        //updateOrientationAngles();
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
                //setMouseDelta(dx, dy);
                break;
        }
        return true;
    }
}
