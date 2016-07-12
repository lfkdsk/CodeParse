package HobbyScript.Parser;

import java.util.List;

/**
 *
 * Created by liufengkai on 16/7/12.
 */
public class BinaryExpr extends AstList {
    public BinaryExpr(List<AstNode> children) {
        super(children);
    }

    public AstNode left() {
        return child(0);
    }

    public String operator() {
        return ((AstLeaf) child(1)).token().getText();
    }

    public AstNode right() {
        return child(1);
    }
}
