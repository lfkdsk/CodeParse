package inter;

import symbols.Type;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Do extends Stmt {
    Expr expr;
    Stmt stmt;

    public Do() {
        expr = null;
        stmt = null;
    }


    public void init(Stmt s, Expr x) {
        expr = x;
        stmt = s;
        if (expr.type != Type.Bool) expr.error("boolean required in do");
    }

    @Override
    public void gen(int th, int nx) {
        after = nx;
        int label = newlabel();
        stmt.gen(th, label);
        emitlabel(label);
        expr.jumping(th, 0);
    }
}
