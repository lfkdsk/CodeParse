package HobbyScript.Literal;

import HobbyScript.Parser.AstLeaf;
import HobbyScript.Token.HobbyToken;

/**
 * Number字面量
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class Number extends AstLeaf {
    public Number(HobbyToken token) {
        super(token);
    }

    public double value() {
        return token.getNumber();
    }

}
