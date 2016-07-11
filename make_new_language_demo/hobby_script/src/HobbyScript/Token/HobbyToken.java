package HobbyScript.Token;

import HobbyScript.Exception.HobbyException;

/**
 * HobbyToken
 * Hobby的脚本版本支持的Token类型很少
 * 暂时支持数字/标示符/字符串
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public abstract class HobbyToken {
    /**
     * End of file
     */
    public static final HobbyToken EOF = new HobbyToken(-1) {
    };

    /**
     * End of line
     */
    public static final String EOL = "\\n";

    private int lineNumber;

    public HobbyToken(int lineNumber) {
        this.lineNumber = lineNumber;
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

    public double getNumber() {
        throw new HobbyException("not number token");
    }

    public String getText() {
        return "";
    }
}
