package HobbyScript.Ast;

import HobbyScript.Eval.EnvironmentCallBack;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * Created by liufengkai on 16/7/12.
 */
public class BinaryExpr extends AstList {

    public BinaryExpr(List<AstNode> children) {
        super(children, HobbyToken.BINARY);
    }

    public AstNode left() {
        return child(0);
    }

    public String operator() {
        return ((AstLeaf) child(1)).token().getText();
    }

    public AstNode right() {
        return child(2);
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.binaryEval(env, this);
    }
}
