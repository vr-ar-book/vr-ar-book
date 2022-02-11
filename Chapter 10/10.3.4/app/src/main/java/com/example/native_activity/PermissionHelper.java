package com.example.native_activity;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.content.ContextCompat;
import android.util.Log;

import java.lang.reflect.Method;

public class PermissionHelper
{
    private static final String LOG_TAG = "PermissionHelper";

    // -----------------------------------------------------------------------------

    public static Activity GetForegroundActivity()
    {
        try
        {
            Class<?> Clazz = Class.forName("com.example.native_activity.GameActivity");

            Method GetInstanceMethod = Clazz.getMethod("GetInstance", new Class[] {});

            return (Activity)GetInstanceMethod.invoke(null);
        }
        catch (Exception _Exception)
        {
            Log.e(LOG_TAG, "GameActivity.GetInstance() failed");
        }

        return null;
    }

    // -----------------------------------------------------------------------------

    public static boolean CheckPermission(String _Permission)
    {
        Activity ForegroundActivity = GetForegroundActivity();

        if (ForegroundActivity == null) return false;

        if (ContextCompat.checkSelfPermission(ForegroundActivity, _Permission) == PackageManager.PERMISSION_GRANTED)
        {
            Log.d(LOG_TAG, "CheckPermission: " + _Permission + " has granted");

            return true;
        }
        else
        {
            Log.d(LOG_TAG, "CheckPermission: " + _Permission + " has not granted");

            return false;
        }
    }

    // -----------------------------------------------------------------------------

    public static void AcquirePermissions(final String _Permissions[])
    {
        Activity ForegroundActivity = GetForegroundActivity();

        PermissionHelper.AcquirePermissions(_Permissions, ForegroundActivity);
    }

    // -----------------------------------------------------------------------------

    public static void AcquirePermissions(final String _Permissions[], Activity _Activity)
    {
        if (_Activity == null) return;

        final Activity FinalActivity = _Activity;

        FinalActivity.runOnUiThread(new Runnable()
        {
            public void run()
            {
                PermissionFragment Fragment = PermissionFragment.GetInstance(FinalActivity);

                if (Fragment != null)
                {
                    Fragment.AcquirePermissions(_Permissions);
                }
            }
        });
    }

    // -----------------------------------------------------------------------------

    public static native void nativeOnAcquirePermissions(String _Permissions[], int _GrantResults[]);
}
