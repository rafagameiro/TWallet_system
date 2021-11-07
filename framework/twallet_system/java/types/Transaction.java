package com.android.myapplication.types;

import com.google.gson.annotations.SerializedName;

public class Transaction {

    @SerializedName("blockNumber")
    private String blkNumber;

    private String timeStamp;

    @SerializedName("hash")
    private String txHash;

    private String nonce;

    @SerializedName("blockHash")
    private String blkHash;

    @SerializedName("transactionIndex")
    private String txIndex;

    private String from;

    private String to;

    private String value;

    private String gas;

    private String gasPrice;

    private String isError;

    @SerializedName("txreceipt_status")
    private String txReceiptStatus;

    private String input;

    private String contractAddress;

    private String cumulativeGasUsed;

    private String gasUsed;

    private String confirmations;

    public Transaction() {
    }

    public String getBlkNumber() {
        return blkNumber;
    }

    public String getTimeStamp() {
        return timeStamp;
    }

    public String getTxHash() {
        return txHash;
    }

    public String getNonce() {
        return nonce;
    }

    public String getBlkHash() {
        return blkHash;
    }

    public String getTxIndex() {
        return txIndex;
    }

    public String getFrom() {
        return from;
    }

    public String getTo() {
        return to;
    }

    public String getValue() {
        return value;
    }

    public String getGas() {
        return gas;
    }

    public String getGasPrice() {
        return gasPrice;
    }

    public String getIsError() {
        return isError;
    }

    public String getTxReceiptStatus() {
        return txReceiptStatus;
    }

    public String getInput() {
        return input;
    }

    public String getContractAddress() {
        return contractAddress;
    }

    public String getCumulativeGasUsed() {
        return cumulativeGasUsed;
    }

    public String getGasUsed() {
        return gasUsed;
    }

    public String getConfirmations() {
        return confirmations;
    }
}
