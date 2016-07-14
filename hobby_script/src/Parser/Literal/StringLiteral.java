package Parser.Literal;

import Parser.Ast.AstLeaf;
import Parser.Eval.EnvironmentCallBack;
import Parser.Eval.ScriptEval;
import Parser.Token.HobbyToken;

/**
 * String
 * Created by liufengkai on 16/7/12.
 */
public class StringLiteral extends AstLeaf {
    public StringLiteral(HobbyToken token) {
        super(token);
    }

    public String value() {
        return token.getText();
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.StringEval(this);
    }
}
