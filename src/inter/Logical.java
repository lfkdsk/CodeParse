package inter;

import lexer.Token;
import symbols.Type;

/**
 * 提供与或非的功能
 * Created by liufengkai on 16/3/17.
 */
public class Logical extends Expr {
    public Expr expr1, expr2;

    public Logical(Token op, Expr expr1, Expr expr2) {
        super(op, null);
        this.expr1 = expr1;
        this.expr2 = expr2;
        this.type = check(expr1.type, expr2.type);
        if (type == null) error("type error");
    }

    /**
     * 类型检查
     *
     * @param type1
     * @param type2
     * @return
     */
    public Type check(Type type1, Type type2) {
        if (type1 == Type.Bool && type2 == Type.Bool)
            return Type.Bool;
        else
            return null;
    }

    @Override
    public Expr gen() {
        int f = newLabel();
        int a = newLabel();
        Temp temp = new Temp(type);
        this.jumping(0, f);
        emit(temp.toString() + " = true");
        emit("goto L" + a);
        emitLabel(f);
        emit(temp.toString() + " = false");
        emitLabel(a);
        return temp;
    }

    public String toString() {
        return expr1.toString() + " " + op.toString() + " " + expr2.toString();
    }
}
