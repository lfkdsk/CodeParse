package HobbyScript.Ast;

import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.FunctionEval;

import java.util.List;

/**
 * 调用参数段
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class Arguments extends Postfix {
    public Arguments(List<AstNode> children) {
        super(children);
    }

    @Override
    public Object eval(EnvironmentCallBack env, Object value) {
        return FunctionEval.argumentsEval(env, this, value);
    }


    public int size() {
        return childCount();
    }
}
