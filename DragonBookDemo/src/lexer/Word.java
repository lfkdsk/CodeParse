package lexer;

/**
 * 词素:保留字\表示符\复合词素
 * Created by liufengkai on 16/3/16.
 */
public class Word extends Token {
    public String lexeme = "";

    public Word(String s, int tag) {
        super(tag);
        lexeme = s;
    }

    public String toString() {
        return lexeme;
    }

    // 定义了一些保留字常量
    public static final Word
            and = new Word("&&", Tag.AND),
            or = new Word("||", Tag.OR),
            eq = new Word("==", Tag.EQ),
            ne = new Word("!=", Tag.NE),
            le = new Word("<=", Tag.LE),
            ge = new Word(">=", Tag.GE),
            minus = new Word("minus", Tag.MINUS),
            True = new Word("true", Tag.TRUE),
            False = new Word("false", Tag.FALSE),
            temp = new Word("t", Tag.TEMP);
}
