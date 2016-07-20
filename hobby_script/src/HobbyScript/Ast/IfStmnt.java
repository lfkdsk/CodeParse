package HobbyScript.Ast;

import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * IF 控制块
 * Created by liufengkai on 16/7/12.
 */
public class IfStmnt extends AstList {

    public IfStmnt(List<AstNode> children) {
        super(children, HobbyToken.IF);
    }

    public AstNode condition() {
        return child(0);
    }

    public AstNode thenBlock() {
        return child(1);
    }

    public AstNode elseBlock() {
        return childCount() > 2 ? child(2) : null;
    }

    public String toString() {
        return "(if " + condition() + " " +
                thenBlock() + " else " +
                elseBlock() + " )";
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.ifEval(env, new LocalEnvironment(), this);
    }
}
