package HobbyScript.Token;

/**
 * 静态类型的类型声明
 *
 * @author liufengkai
 *         Created by liufengkai on 16/8/8.
 */
public class TypeToken extends HobbyToken {

    public static final String INT = "Int",
            FLOAT = "Float",
            STRING = "String";

    public TypeToken(int lineNumber, int type) {
        super(lineNumber, type);
    }

    public boolean isType() {
        return true;
    }
}
