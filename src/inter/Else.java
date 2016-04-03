package inter;

import symbols.Type;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Else extends Stmt {
    Expr expr;
    Stmt stmt1, stmt2;

    public Else(Expr expr, Stmt stmt1, Stmt stmt2) {
        this.expr = expr;
        this.stmt1 = stmt1;
        this.stmt2 = stmt2;
        if (expr.type != Type.Bool) expr.error(" boolean required in if");
    }

    @Override
    public void gen(int th, int nx) {
        int label_1 = newLabel();
        int label_2 = newLabel();

        expr.jumping(0, label_2);

        emitLabel(label_1);
        stmt1.gen(label_1, nx);
        emit("goto L" + nx);

        emitLabel(label_2);
        stmt2.gen(label_2, nx);
    }
}
