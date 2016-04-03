package inter;

import lexer.Token;
import symbols.Type;

/**
 * 双目运算符
 * Created by liufengkai on 16/3/16.
 */
public class Arith extends OP {
    public Expr expr1, expr2;

    /**
     *
     * @param op 运算符词法单元
     * @param expr1
     * @param expr2
     */
    public Arith(Token op, Expr expr1, Expr expr2) {
        super(op, null);
        this.expr1 = expr1;
        this.expr2 = expr2;
        // 计算真实类别
        this.type = Type.max(expr1.type, expr2.type);
        if (type == null) error("type error");
    }

    /**
     * 规约表达式,生成三地址指令的右部
     * @return
     */
    @Override
    public Expr gen() {
        return new Arith(op, expr1.reduce(), expr2.reduce());
    }

    public String toString() {
        return expr1.toString() + " " + op.toString() + " " + expr2.toString();
    }
}
