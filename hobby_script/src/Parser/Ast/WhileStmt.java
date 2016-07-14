package Parser.Ast;

import Parser.Eval.EnvironmentCallBack;

import java.util.List;

/**
 * While控制块
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class WhileStmt extends AstList {
    public WhileStmt(List<AstNode> children) {
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

    @Override
    public Object eval(EnvironmentCallBack env) {
        return super.eval(env);
    }
}
