package symbols;

import lexer.Tag;

/**
 * Created by liufengkai on 16/3/16.
 */
public class Array extends Type {
    // 类型
    public Type of;
    // 元素个数
    public int size = 1;

    public Array(int size, Type type) {
        super("[]", Tag.INDEX, size * type.width);
        this.size = size;
        this.of = type;
    }

    public String toString() {
        return "[" + size + "]" + of.toString();
    }
}
