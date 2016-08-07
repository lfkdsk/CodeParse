package HobbyScript.Literal;

import HobbyScript.Ast.AstLeaf;
import HobbyScript.Compile.CodeLine;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Token.NumberToken;

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

    @Override
    public String compile(CodeLine line, int start, int end) {
        return String.valueOf(((NumberToken) token).getNumber());
    }
}
