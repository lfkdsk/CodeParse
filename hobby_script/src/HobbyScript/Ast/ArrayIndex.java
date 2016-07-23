package HobbyScript.Ast;

import HobbyScript.Eval.ArrayEval;
import HobbyScript.Eval.Env.EnvironmentCallBack;

import java.util.List;

/**
 * 数组
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/23.
 */
public class ArrayIndex extends Postfix {

    public ArrayIndex(List<AstNode> children) {
        super(children);
    }

    public AstNode index() {
        return child(0);
    }

    @Override
    public String toString() {
        return "[ " + index() + " ] ";
    }

    @Override
    public Object eval(EnvironmentCallBack env, Object value) {
        return ArrayEval.arrayIndexEval(env, this, value);
    }
}
