package HobbyScript.Literal;

import HobbyScript.Parser.AstLeaf;
import HobbyScript.Token.HobbyToken;

/**
 * ID 变量
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class Id extends AstLeaf {
    public Id(HobbyToken token) {
        super(token);
    }

    public String name() {
        return token().getText();
    }
}
