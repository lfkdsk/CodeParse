package HobbyScript.Literal;

import HobbyScript.Parser.AstLeaf;
import HobbyScript.Token.HobbyToken;

/**
 * Number字面量
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class NumberLiteral extends AstLeaf {

    public NumberLiteral(HobbyToken token) {
        super(token);
    }

    public int getTag() {
        return token.getTag();
    }
}
