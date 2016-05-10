package inter;

import symbols.Type;

/**
 * DO
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
        after = nx;                 // 保存和while相类
        int label = newLabel();     // 新建
        stmt.gen(th, label);        // 运行啊运行
        emitLabel(label);
        expr.jumping(th, 0);        // 就tmd跳了
    }
}
