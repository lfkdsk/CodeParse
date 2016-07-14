package Parser.Ast;

import Parser.Eval.EnvironmentCallBack;

import java.util.List;

/**
 * IF 控制块
 * Created by liufengkai on 16/7/12.
 */
public class IfStmnt extends AstList {

    public IfStmnt(List<AstNode> children) {
        super(children);
    }

    public AstNode condition() {
        return child(0);
    }

    public AstNode theBlock() {
        return child(1);
    }

    public AstNode elseBlock() {
        return childCount() > 2 ? child(2) : null;
    }

    public String toString() {
        return "(if " + condition() + " " +
                theBlock() + " else " +
                elseBlock() + " )";
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return super.eval(env);
    }
}
