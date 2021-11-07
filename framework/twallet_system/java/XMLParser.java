package android.application.path;

import path.to.type.types.AccountInfo;

import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.StringWriter;


public class XMLParser {


    public static String parseTo(AccountInfo info) {

        String xml = null;
        try {
            Serializer serializer = new Persister();
            StringWriter sw = new StringWriter();
            serializer.write(info, sw);
            xml = sw.toString();

        } catch (Exception ignored) {
        }

        return xml;
    }

    public static AccountInfo parseFrom(String xml) {

        AccountInfo info = new AccountInfo();
        if (xml != null)
            try {
                Serializer serializer = new Persister();
                info = serializer.read(AccountInfo.class, xml);

                return info;
            } catch (Exception ignored) {
            }

        return info;
    }

}
