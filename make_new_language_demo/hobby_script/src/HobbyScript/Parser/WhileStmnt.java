package HobbyScript.Parser;

import HobbyScript.Parser.AstList;
import HobbyScript.Parser.AstNode;

import java.util.List;

/**
 * While控制块
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class WhileStmnt extends AstList {
    public WhileStmnt(List<AstNode> children) {
        super(children);
    }

    public AstNode condition() {
        return child(0);
    }

    public AstNode body() {
        return child(1);
    }

    public String toString() {
        return "(while " + condition() + " " + body() + ")";
    }
}
