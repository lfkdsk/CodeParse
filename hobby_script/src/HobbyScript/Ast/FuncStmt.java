package HobbyScript.Ast;

import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * Created by liufengkai on 16/7/15.
 */
public class FuncStmt extends AstList {
    public FuncStmt(List<AstNode> children) {
        super(children, HobbyToken.FUNCTION);
    }
}
