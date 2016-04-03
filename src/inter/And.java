package inter;

import lexer.Token;

/**
 * Add
 * Created by liufengkai on 16/3/17.
 */
public class And extends Logical {
    public And(Token op, Expr expr1, Expr expr2) {
        super(op, expr1, expr2);
    }

    @Override
    public void jumping(int t, int f) {
        // and 运算和B2关系较大
        int label = f != 0 ? f : newLabel();
        expr1.jumping(0, label);
        expr2.jumping(t, f);
        if (f == 0) emitLabel(label);
    }
}
