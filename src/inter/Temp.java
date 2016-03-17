package inter;

import lexer.Word;
import symbols.Type;

/**
 * 临时名字
 * Created by liufengkai on 16/3/16.
 */
public class Temp extends Expr {
    static int count = 0;

    int number = 0;

    public Temp(Type type) {
        super(Word.temp, type);
        number = ++count;
    }

    public String toString() {
        return "t" + number;
    }
}
