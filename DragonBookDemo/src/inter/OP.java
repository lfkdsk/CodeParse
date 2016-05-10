package inter;

import lexer.Token;
import symbols.Type;

/**
 * 运算符
 * // 属于reduce()的一种实现 就生成临时变量并返回
 * 子类有Arith(算术运算符),Unary单目运算符,Access数组指向运算
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
