package symbols;

import lexer.Tag;
import lexer.Word;

/**
 * Created by liufengkai on 16/3/16.
 */
public class Type extends Word {
    public int width = 0;

    public Type(String s, int tag, int width) {
        super(s, tag);
        this.width = width;
    }


    // 定义基本类型
    public static final Type
            Int = new Type("int", Tag.BASIC, 4),
            Float = new Type("float", Tag.BASIC, 8),
            Char = new Type("char", Tag.BASIC, 1),
            Bool = new Type("bool", Tag.BASIC, 1);

    // 判断两数运算时候最大的类型
    public static boolean numeric(Type type) {
        return type == Type.Char || type == Type.Int || type == Type.Float;
    }

    public static Type max(Type p1, Type p2) {
        if (!numeric(p1) || !numeric(p2)) return null;
        else if (p1 == Type.Float || p2 == Type.Float) return Type.Float;
        else if (p1 == Type.Int || p2 == Type.Int) return Type.Int;
        else return Type.Char;
    }

}
