package HobbyScript.Token;

/**
 * Null
 * 
 * Created by liufengkai on 16/8/4.
 */
public class NullToken extends HobbyToken {
    public NullToken(int lineNumber) {
        super(lineNumber, HobbyToken.NULL);
    }

    @Override
    public boolean isNull() {
        return true;
    }
}
