package lexer;

/**
 * 浮点处理类
 * Created by liufengkai on 16/3/16.
 */
public class Real extends Token {
    public final float value;

    public Real(float value) {
        super(Tag.REAL);
        this.value = value;
    }

    public String toString() {
        return "" + value;
    }
}
