package HobbyScript.Parser;

import java.util.List;

/**
 * Created by liufengkai on 16/7/12.
 */
public class NegativeExpr extends AstList {
    public NegativeExpr(List<AstNode> children) {
        super(children);
    }

    public AstNode operand() {
        return child(0);
    }

    public String toString() {
        return "-" + operand();
    }
}
