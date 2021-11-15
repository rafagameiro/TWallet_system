# TWallet System

The purpose of this repository is to provide the code for the solution developed throughout my MSc Thesis, the TWallet System.

## Folder structure

This repository is organized as the following:

- Framework: contains the files that can be added to the mobile applications so that they can make use of the TWallet System.
- Components: contains the Trusted Applications that will run inside the ARM TrustZone and provide additional security and trustability guarantees to the applications that make use of their functionalities.

## Pre-requisites

To build the developed Secure Components, you must have the OP-TEE OS[[1]](#references) cloned and built

The instructions regarding the TWallet Framework take into account the normal setup of an Android App developed using the Android Studio [[2]](#references) and therefore, its advisable to use when adding our solution.

## Build setup
### Secure Components

To build the Secure Components, run the following command for each secure component:
```
make \
    CROSS_COMPILE=arm-linux-gnueabihf- \
    PLATFORM=hikey960 \
    TA_DEV_KIT_DIR=<optee_os>/out/arm/export-ta_arm32
```

To add the Secure Components executables to your system, you can follow two processes:
1. You can add the Trusted Application .ta generated file to the AOSP+OP-TEE files and then regenerate the system
2. You can push the .ta file to the system specific folder using ADB, using the commands:
```
adb root
adb remount
adb push [name-of-file].ta /vendor/lib/optee_armtz/
adb reboot
```

### TWallet Framework

To build the TWallet Framework:
 - Add files inside the cpp folder (not the folder itself) to your project `app/app/src/cpp`
 - Add the files inside the Twallet folder into your project solution as a package

After that you can use the functions made available by the TWallet Framework.

## References

1. Linaro. OP-TEE OS. 2021. url: https://optee.readthedocs.io/en/latest/building/gits/optee_os.html#optee-os
2. Android Developers. Android Studio 2020.3.1. 2021. url: https://developer.android.com/studio/