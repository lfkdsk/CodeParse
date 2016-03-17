package inter;

import lexer.Token;
import symbols.Type;

/**
 * Created by liufengkai on 16/3/16.
 */
public class ID extends Expr {
    // 相对地址
    public int offset;

    public ID(Token op, Type type, int b) {
        super(op, type);
        this.offset = b;
    }
}
