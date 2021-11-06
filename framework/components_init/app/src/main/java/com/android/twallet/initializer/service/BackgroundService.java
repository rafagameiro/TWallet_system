package com.android.twallet.initializer.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class BackgroundService extends Service {
    private static final String TAG = "backgroundService";

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void onDestroy() {
        Log.d(TAG, "onDestroy");
    }

    @Override
    public void onStart(Intent intent, int startid) {
        ComponentsLauncher launcher = new ComponentsLauncher();
        launcher.launchComponents();
        Log.d(TAG, "onStart");
    }
}
