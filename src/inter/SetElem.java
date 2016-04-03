package inter;

import symbols.Array;
import symbols.Type;

/**
 * 数组元素赋值
 *
 * Created by liufengkai on 16/3/17.
 */
public class SetElem extends Stmt {
    // array 标示符
    public ID array;
    public Expr index;
    public Expr expr;

    public SetElem(Access x, Expr y) {
        this.array = x.array;
        this.index = x.index;
        this.expr = y;

        if (check(x.type, expr.type) == null) error("type error");
    }

    public Type check(Type p1, Type p2) {
        if (p1 instanceof Array || p2 instanceof Array) return null;
        else if (p1 == p2) return p2;
        else if (Type.numeric(p1) && Type.numeric(p2)) return p2;
        else return null;
    }

    @Override
    public void gen(int th, int nx) {
        String s1 = index.reduce().toString();
        String s2 = expr.reduce().toString();
        emit(array.toString() + " [ " + s1 + " ] = " + s2);
    }
}
