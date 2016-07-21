package HobbyScript.Literal;

import HobbyScript.Ast.BlockStmnt;
import HobbyScript.Ast.ParameterList;
import HobbyScript.Eval.Env.EnvironmentCallBack;

/**
 * Created by liufengkai on 16/7/22.
 */
public class ClassFunction extends Function {
    public ClassFunction(ParameterList parameters, BlockStmnt body, EnvironmentCallBack env) {
        super(parameters, body, env);
    }
}
