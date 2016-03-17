package inter;

import lexer.Num;
import lexer.Token;
import lexer.Word;
import symbols.Type;

/**
 * 布尔表达式的跳转指令生成
 * Created by liufengkai on 16/3/16.
 */
public class Constant extends Expr {
    public Constant(Token op, Type type) {
        super(op, type);
    }

    public Constant(int i) {
        super(new Num(i), Type.Int);
    }

    public static final Constant
            True = new Constant(Word.True, Type.Bool),
            False = new Constant(Word.False, Type.Bool);

    public void jumping(int t, int f) {
        if (this == True && t != 0) emit("goto L" + t);
        else if (this == False && f != 0) emit("goto L" + f);
    }
}
