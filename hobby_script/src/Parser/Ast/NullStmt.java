package Parser.Ast;

import Parser.Eval.EnvironmentCallBack;

import java.util.List;

/**
 * 空
 * Created by liufengkai on 16/7/12.
 */
public class NullStmt extends AstList {
    public NullStmt(List<AstNode> children) {
        super(children);
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return super.eval(env);
    }
}
