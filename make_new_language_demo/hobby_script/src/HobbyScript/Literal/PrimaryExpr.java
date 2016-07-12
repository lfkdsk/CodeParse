package HobbyScript.Literal;

import HobbyScript.Parser.AstList;
import HobbyScript.Parser.AstNode;

import java.util.List;

/**
 * Primary
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class PrimaryExpr extends AstList {

    public PrimaryExpr(List<AstNode> children) {
        super(children);
    }

    public static AstNode create(List<AstNode> c) {
        return c.size() == 1 ? c.get(0) : new PrimaryExpr(c);
    }
}
