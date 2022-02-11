
package com.example.native_activity;

import android.app.NativeActivity;

import android.content.Context;

import android.os.Bundle;

public class GameActivity extends NativeActivity
{
    // -----------------------------------------------------------------------------
    // Instance
    // -----------------------------------------------------------------------------
    static GameActivity s_Instance;

    public static GameActivity GetInstance()
    {
        return s_Instance;
    }

    // -----------------------------------------------------------------------------
    // App lifecycle
    // -----------------------------------------------------------------------------
    @Override
    public void onCreate(Bundle _SavedInstanceState)
    {
        super.onCreate(_SavedInstanceState);

        s_Instance = this;

        nativeInitializeInterface(getApplicationContext());
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onStart()
    {
        super.onStart();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onResume()
    {
        super.onResume();
    }

    // -----------------------------------------------------------------------------

    @Override
    protected void onPause()
    {
        super.onPause();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onStop()
    {
        super.onStop();
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    // -----------------------------------------------------------------------------
    // Device
    // -----------------------------------------------------------------------------
    public int GetDeviceRotation()
    {
        return getWindowManager().getDefaultDisplay().getRotation();
    }

    // -----------------------------------------------------------------------------
    // Native functions
    // -----------------------------------------------------------------------------
    public native void nativeInitializeInterface(Context context);

    // -----------------------------------------------------------------------------
    // Load libraries. E.g. app with native JNI interface
    // -----------------------------------------------------------------------------
    static
    {
        System.loadLibrary("native-activity");
    }
}
