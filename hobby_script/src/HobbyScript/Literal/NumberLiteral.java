package HobbyScript.Literal;

import HobbyScript.Ast.AstLeaf;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

/**
 * Number字面量
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class NumberLiteral extends AstLeaf {

    public NumberLiteral(HobbyToken token) {
        super(token);
    }

    public int getTag() {
        return token.getTag();
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.NumberEval(this);
    }
}
