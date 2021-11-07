package com.android.myapplication.types;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.ElementList;
import org.simpleframework.xml.Root;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

@Root(name = "account")
public class AccountInfo {

    @Element
    protected BigInteger balance;

    @ElementList
    protected List<Transaction> transactions;

    public AccountInfo() {
        this.transactions = new ArrayList<>();
        this.balance = new BigInteger("0");
    }

    public AccountInfo(List<Transaction> transactions, BigInteger balance) {
        this.transactions = transactions;
        this.balance = balance;
    }

    public List<Transaction> getTransactions() {
        return transactions;
    }

    public BigInteger getBalance() {
        return balance;
    }

    public boolean isEmpty() {
        return balance.intValue() == 0 && transactions == null;
    }
}
