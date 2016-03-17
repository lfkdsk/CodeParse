package inter;

import lexer.Tag;
import lexer.Word;
import symbols.Type;

/**
 * 布尔表达式出现数组
 * Created by liufengkai on 16/3/17.
 */
public class Access extends OP {
    public ID array;
    public Expr index;

    public Access(ID id, Expr expr, Type type) {
        super(new Word("[]", Tag.INDEX), type);
        this.array = id;
        this.index = expr;
    }

    @Override
    public Expr gen() {
        return new Access(array, index.reduce(), type);
    }

    @Override
    public void jumping(int t, int f) {
        emitjumps(reduce().toString(), t, f);
    }

    public String toString() {
        return array.toString() + " [ " + index.toString() + " ] ";
    }
}
