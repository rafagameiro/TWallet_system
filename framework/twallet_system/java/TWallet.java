package com.android.myapplication;

import com.android.myapplication.types.AccountInfo;

public interface TWallet {

    public boolean attestComponents(int nonce);

    public boolean storeCredentials(String id, String password);

    public String[] loadCredentials();

    public boolean deleteCredentials();

    public AccountInfo readData(String id);

    public boolean writeData(String id, AccountInfo info);

    public boolean deleteData(String id);

    public String getLoggingData();

    public boolean seTrigger(boolean trigger);
}
