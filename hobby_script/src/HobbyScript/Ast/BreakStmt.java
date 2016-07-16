package HobbyScript.Ast;

import HobbyScript.Eval.EnvironmentCallBack;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * Break 语句
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/16.
 */
public class BreakStmt extends AstList {

    public BreakStmt(List<AstNode> children) {
        super(children, HobbyToken.BREAK);
    }

    // 携带的返回值
    protected Object result;

    public Object getResult() {
        return result;
    }

    public Object setResult(Object result) {
        this.result = result;
        return this;
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return new Object();
    }
}
