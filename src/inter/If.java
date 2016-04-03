package inter;

import symbols.Type;

/**
 * Created by liufengkai on 16/3/17.
 */
public class If extends Stmt {
    Expr expr;
    Stmt stmt;

    public If(Expr expr, Stmt stmt) {
        this.expr = expr;
        this.stmt = stmt;
        if (expr.type != Type.Bool) expr.error("boolean required in if");
    }

    @Override
    public void gen(int th, int nx) {
        // stmt 标号
        int label = newLabel();
        // jump
        expr.jumping(0, nx);
        emitLabel(label);
        stmt.gen(label, nx);
    }
}
