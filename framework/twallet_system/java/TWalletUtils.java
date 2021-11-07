package com.android.myapplication;

import com.android.myapplication.types.AccountInfo;

public class TWalletUtils implements TWallet {

    private static final String DEVICE_ID = "32465c86-5f3e-4290-94e6-283544a0237b";

    static {
        System.loadLibrary("twallet");
    }

    public TWalletUtils() {
    }

    public boolean attestComponents(int nonce) {
        String[] values = attestComponentsTA(nonce);
        return AttestationValidator.validateProof(values[0], values[1], values[2], values[3]);
    }

    /* Calls storeCredentialsTA method, using the AUTH_ID as serviceID */
    public boolean storeCredentials(String id, String password) {
        return storeCredentialsTA(DEVICE_ID, id, password);
    }

    /* Calls loadCredentialsTA method, using the AUTH_ID as serviceID */
    public String[] loadCredentials() {
        return loadCredentialsTA(DEVICE_ID);
    }

    /* Calls loadCredentialsTA method, using the AUTH_ID as serviceID */
    public boolean deleteCredentials() {
        return deleteCredentialsTA(DEVICE_ID);
    }

    public AccountInfo readData(String id) {
        return XMLParser.parseFrom(readDataTA(id));
    }

    public boolean writeData(String id, AccountInfo info) {
        return writeDataTA(id, XMLParser.parseTo(info));
    }

    public boolean deleteData(String id) {
        return deleteDataTA(id);
    }

    public String getLoggingData() {
        return getLoggingDataTA();
    }

    public boolean seTrigger(boolean trigger) {
        return setTriggerTA(trigger);
    }

    /*---------------------------------- Native Methods-------------------------------------------*/

    /* Attests Components and verifies if all are secure */
    public static native String[] attestComponentsTA(int nonce);

    /* Writes data into the secure storage component */
    private static native boolean writeDataTA(String id, String content);

    /* Read data from the secure storage component using string id */
    private static native String readDataTA(String id);

    /* Deletes data from the secure storage component using string id */
    private static native boolean deleteDataTA(String id);

    /* Stores a new credentials entry into the authentication service component */
    private static native boolean storeCredentialsTA(String serviceID, String id, String password);

    /* Loads the credentials entry stored in the authentication service component */
    private static native String[] loadCredentialsTA(String serviceID);

    /* Loads the credentials entry stored in the authentication service component */
    private static native boolean deleteCredentialsTA(String serviceID);

    /* Retrieves the generated log, stored inside the Secure World */
    private static native String getLoggingDataTA();

    /* Changes the filtering behaviour, depending on the trigger value */
    private static native boolean setTriggerTA(boolean trigger);
}
