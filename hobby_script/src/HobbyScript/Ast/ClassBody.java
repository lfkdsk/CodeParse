package HobbyScript.Ast;

import HobbyScript.Eval.ClassEval;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * 类的定义体
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/20.
 */
public class ClassBody extends AstList {
    // 类体的环境
    private LocalEnvironment env;

    public LocalEnvironment getEnv() {
        return env;
    }

    public void setEnv(LocalEnvironment env) {
        this.env = env;
    }

    public ClassBody(List<AstNode> children) {
        super(children, HobbyToken.CLASS_BODY_TOKEN);
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ClassEval.classBodyEval(env, this);
    }
}
