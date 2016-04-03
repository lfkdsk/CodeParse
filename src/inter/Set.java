package inter;

import symbols.Type;

/**
 * Set赋值
 * id = expr;
 * Created by liufengkai on 16/3/17.
 */
public class Set extends Stmt {
    // 标示
    public ID id;
    // 表达式
    public Expr expr;

    public Set(ID id, Expr expr) {
        this.id = id;
        this.expr = expr;
        if (check(id.type, expr.type) == null) error("type error");
    }

    public Type check(Type p1, Type p2) {
        if (Type.numeric(p1) && Type.numeric(p2))
            return p2;
        else if (p1 == Type.Bool && p2 == Type.Bool)
            return p2;
        else return null;
    }

    @Override
    public void gen(int th, int nx) {
        emit(id.toString() + " = " + expr.gen().toString());
    }
}
