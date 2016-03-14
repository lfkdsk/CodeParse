package lexer;

/**
 * Created by liufengkai on 16/3/13.
 */
public class Token {
    public final int tag;

    public Token(int t) {
        tag = t;
    }

    public String toString() {
        return "" + (char) tag;
    }
}
