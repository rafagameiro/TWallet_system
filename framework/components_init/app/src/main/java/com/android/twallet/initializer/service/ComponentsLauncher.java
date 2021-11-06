package com.android.twallet.initializer.service;

public class ComponentsLauncher {

    static {
        System.loadLibrary("initializer");
    }

    public ComponentsLauncher() {

    }

    public void launchComponents() {
        launchComponentsTA();
    }

    private static native void launchComponentsTA();
}
