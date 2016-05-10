package inter;

import symbols.Type;

/**
 * IF
 *
 * exp: If( E ) S
 * Created by liufengkai on 16/3/17.
 */
public class If extends Stmt {
    // 保存 E
    Expr expr;
    // 保存 S
    Stmt stmt;

    public If(Expr expr, Stmt stmt) {
        this.expr = expr;
        this.stmt = stmt;
        // check type
        if (expr.type != Type.Bool) expr.error("boolean required in if");
    }


    /**
     *
     * @param th
     * @param nx
     */
    @Override
    public void gen(int th, int nx) {
        // stmt 标号
        int label = newLabel();
        // jump
        expr.jumping(0, nx);// 为真时控制穿越流,为假时转向nx
        emitLabel(label);
        stmt.gen(label, nx);
    }
}
