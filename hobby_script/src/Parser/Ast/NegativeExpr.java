package Parser.Ast;

import Parser.Eval.EnvironmentCallBack;

import java.util.List;

/**
 * 正负
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
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

    @Override
    public Object eval(EnvironmentCallBack env) {
        return super.eval(env);
    }
}
