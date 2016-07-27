package HobbyScript.Literal;

import HobbyScript.Ast.AstLeaf;
import HobbyScript.Token.BoolToken;
import HobbyScript.Token.HobbyToken;

/**
 * BoolLiteral
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/27.
 */
public class BoolLiteral extends AstLeaf {

    public BoolLiteral(HobbyToken token) {
        super(token);
    }

    public int tag() {
        return token.getTag();
    }

    public boolean value() {
        return ((BoolToken) token).getType() == BoolToken.BoolType.TRUE;
    }

}
