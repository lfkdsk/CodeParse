package lexer;

/**
 * 数字标识
 * Created by liufengkai on 16/3/16.
 */
public class Num extends Token {
    public final int value;

    public Num(int value) {
        super(Tag.NUM);
        this.value = value;
    }

    public String toString() {
        return "" + value;
    }

}
