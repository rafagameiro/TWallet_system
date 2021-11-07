package com.android.myapplication;

import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.RSAPublicKeySpec;
import java.util.Base64;

public class AttestationValidator {

    private static final String SIGN_ALGORITHM = "SHA256withRSA";

    public static boolean validateProof(String digest, String proof, String exp, String modulus) {

        Base64.Decoder decoder = Base64.getDecoder();

        try {
            BigInteger modulusBig = new BigInteger(-1, decoder.decode(modulus));
            BigInteger expBig = new BigInteger(exp);

            RSAPublicKeySpec spec = new RSAPublicKeySpec(modulusBig.abs(), expBig);

            KeyFactory factory = KeyFactory.getInstance("RSA");
            PublicKey pub = factory.generatePublic(spec);
            Signature sign = Signature.getInstance(SIGN_ALGORITHM);

            sign.initVerify(pub);

            sign.update(decoder.decode(digest));

            return sign.verify(decoder.decode(proof));

        } catch (Exception e) {
            return false;
        }
    }
}
