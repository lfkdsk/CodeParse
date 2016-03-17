package inter;

import symbols.Type;

/**
 * Created by liufengkai on 16/3/17.
 */
public class While extends Stmt {
    Expr expr;
    Stmt stmt;

    public While() {
        expr = null;
        stmt = null;
    }

    public void init(Expr x, Stmt s) {
        expr = x;
        stmt = s;
        if (expr.type != Type.Bool) expr.error("boolean required in while");
    }

    @Override
    public void gen(int th, int nx) {
        after = nx;
        expr.jumping(0, nx);
        int label = newlabel();
        emitlabel(label);
        stmt.gen(label, th);
        emit("goto L" + th);
    }
}
