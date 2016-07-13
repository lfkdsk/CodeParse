package HobbyScript.Token;

/**
 * HobbyToken
 * Hobby的脚本版本支持的Token类型很少
 * 暂时支持数字/标示符/字符串
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public abstract class HobbyToken {

    public final static int
            AND = 256, BASIC = 257, BREAK = 258, DO = 259, ELSE = 260,
            EQ = 261, FALSE = 262, GE = 263, ID = 264, IF = 265,
            INDEX = 266, LE = 267, MINUS = 268, NE = 269, NUM = 270,
            OR = 271, REAL = 272, TEMP = 273, TRUE = 274, WHILE = 275,
            STRING = 276, EOF_TAG = -1, EOL_TAG = -2;

    /**
     * End of file
     */
    public static final HobbyToken EOF = new HobbyToken(-1, EOF_TAG){};

    /**
     * End of line
     */
    public static final String EOL = "\\n";


    private int lineNumber;

    protected int tag;

    public HobbyToken(int lineNumber, int tag) {
        this.lineNumber = lineNumber;
        this.tag = tag;
    }


    public int getLineNumber() {
        return lineNumber;
    }

    public boolean isIdentifier() {
        return false;
    }

    public boolean isNumber() {
        return false;
    }

    public boolean isString() {
        return false;
    }

    public String getText() {
        return "";
    }

    public int getTag() {
        return tag;
    }
}
