package inter;

import lexer.Token;
import symbols.Type;

/**
 * 表达式
 * Created by liufengkai on 16/3/16.
 */
public class Expr extends Node {
    // 运算符
    public Token op;
    // 类型
    public Type type;


    public Expr(Token op, Type type) {
        this.op = op;
        this.type = type;
    }

    public Expr gen() {
        return this;
    }

    public Expr reduce() {
        return this;
    }

    public void jumping(int t, int f) {
        emitjumps(toString(), t, f);
    }

    // 跳转
    public void emitjumps(String tests, int t, int f) {
        if (t != 0 && f != 0) {
            emit("if " + tests + " goto L" + t);
            emit("goto L" + f);
        } else if (t != 0)
            emit("if " + tests + " goto L" + t);
        else if (f != 0)
            emit("iffalse " + tests + "goto L" + f);
        else ;
    }

    public String toString() {
        return op.toString();
    }
}
