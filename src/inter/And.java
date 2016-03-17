package inter;

import lexer.Token;

/**
 * Created by liufengkai on 16/3/17.
 */
public class And extends Logical {
    public And(Token op, Expr expr1, Expr expr2) {
        super(op, expr1, expr2);
    }

    @Override
    public void jumping(int t, int f) {
        int label = f != 0 ? f : newlabel();
        expr1.jumping(0, label);
        expr2.jumping(t, f);
        if (f == 0) emitlabel(label);
    }
}
