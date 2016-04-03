package lexer;

/**
 * 词法元素
 * Created by liufengkai on 16/3/16.
 */
public class Token {
    public final int tag;

    public Token(int tag) {
        this.tag = tag;
    }

    public String toString() {
        return "" + (char) tag;
    }
}
