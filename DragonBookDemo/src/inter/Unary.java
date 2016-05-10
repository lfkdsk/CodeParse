package inter;

import lexer.Token;
import symbols.Type;

/**
 * 单目运算符
 * Created by liufengkai on 16/3/16.
 */
public class Unary extends Expr {
    public Expr expr;

    public Unary(Token op, Expr expr) {
        super(op, null);
        this.expr = expr;
        this.type = Type.max(Type.Int, expr.type);
        if (type == null) error("type error");
    }

    /**
     * 生成三目运算符的右侧地址
     * @return
     */
    @Override
    public Expr gen() {
        return new Unary(op, expr.reduce());
    }

    public String toString() {
        return op.toString() + " " + expr.toString();
    }
}
