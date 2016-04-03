package inter;

import lexer.Token;

/**
 * Not
 * Created by liufengkai on 16/3/17.
 */
public class Not extends Logical {
    public Not(Token op, Expr expr2) {
        super(op, expr2, expr2);
    }

    @Override
    public void jumping(int t, int f) {
        expr2.jumping(f, t);
    }

    @Override
    public String toString() {
        return op.toString() + " " + expr2.toString();
    }
}
