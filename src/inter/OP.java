package inter;

import lexer.Token;
import symbols.Type;

/**
 * 运算符
 * Created by liufengkai on 16/3/16.
 */
public class OP extends Expr {

    public OP(Token op, Type type) {
        super(op, type);
    }

    // 生成临时名字 返回
    public Expr reduce() {
        Expr x = gen();
        Temp t = new Temp(type);
        emit(t.toString() + " = " + x.toString());
        return t;
    }
}
