package inter;

import lexer.Token;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Or extends Logical {
    public Or(Token op, Expr expr1, Expr expr2) {
        super(op, expr1, expr2);
    }

    @Override
    public void jumping(int t, int f) {
        int label = t != 0 ? t : newlabel();
        expr1.jumping(label, 0);
        expr2.jumping(t, f);
        if (t == 0) emitlabel(label);
    }
}
