package com.example.native_activity;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Build;
import android.support.v13.app.FragmentCompat;
import android.util.Log;

public class PermissionFragment extends Fragment implements FragmentCompat.OnRequestPermissionsResultCallback
{
    private static final int PERMISSION_REQUEST_CODE = 1101;
    private static final String TAG = "PermissionFragment";
    private static final String PERMISSION_TAG = "TAG_PermissionFragment";

    // -----------------------------------------------------------------------------

    public static PermissionFragment GetInstance(Activity _Activity)
    {
        FragmentManager ActivityFragmentManager = _Activity.getFragmentManager();

        PermissionFragment Fragment = (PermissionFragment)ActivityFragmentManager.findFragmentByTag(PERMISSION_TAG);

        if (Fragment == null)
        {
            try
            {
                Log.d(TAG, "Creating PermissionFragment");

                Fragment = new PermissionFragment();

                FragmentTransaction Transaction = ActivityFragmentManager.beginTransaction();

                Transaction.add(Fragment, PERMISSION_TAG);

                Transaction.commit();

                ActivityFragmentManager.executePendingTransactions();
            }
            catch (Throwable _Throwable)
            {
                Log.e(TAG, "Can not launch PermissionFragment:" + _Throwable.getMessage(), _Throwable);

                return null;
            }
        }

        return Fragment;
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onCreate(Bundle _SavedInstanceState)
    {
        super.onCreate(_SavedInstanceState);

        setRetainInstance(true);
    }

    // -----------------------------------------------------------------------------

    @Override
    public void onRequestPermissionsResult(int _RequestCode, String[] _Permissions, int[] _GrantResults)
    {
        // -----------------------------------------------------------------------------
        // Only in Marshmallow(23) and above needs runtime permission checking
        // -----------------------------------------------------------------------------
        if (Build.VERSION.SDK_INT >= 23)
        {
            super.onRequestPermissionsResult(_RequestCode, _Permissions, _GrantResults);
        }

        if (_RequestCode == PERMISSION_REQUEST_CODE && _Permissions.length > 0)
        {
            if (_GrantResults.length > 0 && _GrantResults[0] == PackageManager.PERMISSION_GRANTED)
            {
                Log.d(TAG, "Permission granted for " + _Permissions[0]);
            }
            else
            {
                Log.d(TAG, "Permission not granted for " + _Permissions[0]);
            }

            PermissionHelper.nativeOnAcquirePermissions(_Permissions, _GrantResults);
        }
    }

    // -----------------------------------------------------------------------------

    public void AcquirePermissions(String _Permissions[])
    {
        FragmentCompat.requestPermissions(this, _Permissions, PERMISSION_REQUEST_CODE);
    }
}
