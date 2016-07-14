package Parser.Literal;

import Parser.Ast.AstLeaf;
import Parser.Eval.EnvironmentCallBack;
import Parser.Eval.ScriptEval;
import Parser.Token.HobbyToken;

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
