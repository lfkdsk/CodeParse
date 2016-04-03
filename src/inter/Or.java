package inter;

import lexer.Token;

/**
 * OR
 * Created by liufengkai on 16/3/17.
 */
public class Or extends Logical {
    public Or(Token op, Expr expr1, Expr expr2) {
        super(op, expr1, expr2);
    }


    /**
     * B = B1 || B2
     * @param t true 出口
     * @param f false 出口
     */
    @Override
    public void jumping(int t, int f) {
        // 设置B true ---> newLabel ; false ---> t
        int label = t != 0 ? t : newLabel();
        // b1 的true 和 B 相同 false 对应 B2 第一条指令
        expr1.jumping(label, 0);
        // b2 的和B相同
        expr2.jumping(t, f);
        // 结尾处生成新标号
        if (t == 0) emitLabel(label);
    }
}
