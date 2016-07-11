package HobbyScript.Token;

/**
 * 数字的Token
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class NumberToken extends HobbyToken {
    private double value;

    public NumberToken(int lineNumber, double value) {
        super(lineNumber);
        this.value = value;
    }

    @Override
    public boolean isNumber() {
        return true;
    }

    @Override
    public double getNumber() {
        return value;
    }

    @Override
    public String getText() {
        return Double.toString(value);
    }
}
