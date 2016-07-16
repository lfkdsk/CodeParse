package HobbyScript.Ast;

import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * 函数
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class FuncStmt extends AstList {
    public FuncStmt(List<AstNode> children) {
        super(children, HobbyToken.FUNCTION);
    }

    public String name() {
        return ((AstLeaf) child(0)).token().getText();
    }

    public ParameterList parameters() {
        return (ParameterList) child(1);
    }

    public BlockStmnt body() {
        return (BlockStmnt) child(2);
    }

    public String toString() {
        return "(func " + name() + " " + parameters() + " "
                + body() + " )";
    }
}
