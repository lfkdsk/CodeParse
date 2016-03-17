package inter;

import lexer.Token;
import symbols.Array;
import symbols.Type;

/**
 * 处理双目运算符
 * Created by liufengkai on 16/3/17.
 */
public class Rel extends Logical {
    public Rel(Token op, Expr expr1, Expr expr2) {
        super(op, expr1, expr2);
    }

    @Override
    public Type check(Type type1, Type type2) {
        // 类型检查 不允许数组类型进行比较
        if (type1 instanceof Array || type2 instanceof Array) {
            return null;
        } else if (type1 == type2)
            return Type.Bool;
        else return null;
    }

    @Override
    public void jumping(int t, int f) {
        Expr temp1 = expr1.reduce();
        Expr temp2 = expr2.reduce();

        String test = temp1.toString() + " " + op.toString() + " " + temp2.toString();
        emitjumps(test, t, f);
    }
}
