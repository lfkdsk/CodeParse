package HobbyScript.Token;

/**
 * 数字的Token
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class NumberToken<T extends java.lang.Number> extends HobbyToken {

    private T value;

    public NumberToken(int lineNumber, int tag, T value) {
        super(lineNumber, tag);
        this.value = value;
    }

    @Override
    public boolean isNumber() {
        return true;
    }

    public T getNumber() {
        return value;
    }

    @Override
    public String getText() {
        return String.valueOf(value);
    }
}
