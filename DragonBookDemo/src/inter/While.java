package inter;

import symbols.Type;

/**
 * WHILE
 * Created by liufengkai on 16/3/17.
 */
public class While extends Stmt {
    Expr expr;
    Stmt stmt;

    /**
     * 空节点
     */
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
        after = nx;                 // 保存用于跳出的地址
        expr.jumping(0, nx);
        int label = newLabel();     // stmt
        emitLabel(label);
        stmt.gen(label, th);        // 目标为th的跳转指令
        emit("goto L" + th);        // 打印跳转
    }
}
