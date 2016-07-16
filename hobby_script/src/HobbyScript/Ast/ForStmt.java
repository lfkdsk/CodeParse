package HobbyScript.Ast;

import HobbyScript.Eval.EnvironmentCallBack;
import HobbyScript.Eval.LocalEnvironment;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * For
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/16.
 */
public class ForStmt extends AstList {

    public ForStmt(List<AstNode> children) {
        super(children, HobbyToken.FOR);
    }

    public AstNode initial() {
        return child(0);
    }

    public AstNode condition() {
        return child(1);
    }

    public AstNode step() {
        return child(2);
    }

    public AstNode body() {
        return child(3);
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.forEval(env, new LocalEnvironment(), this);
    }
}
