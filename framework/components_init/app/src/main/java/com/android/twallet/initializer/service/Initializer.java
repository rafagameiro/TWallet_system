package com.android.twallet.initializer.service;

import android.app.Activity;
import android.os.Bundle;

public class Initializer extends Activity {

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ComponentsLauncher launcher = new ComponentsLauncher();
        launcher.launchComponents();
        finish();
    }

}
