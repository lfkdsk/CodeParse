package HobbyScript.Ast;

import HobbyScript.Compile.CodeLine;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Literal.IdLiteral;
import HobbyScript.Parser.ScriptParser;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * 处理语句
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class BinaryExpr extends AstList {

    public BinaryExpr(List<AstNode> children) {
        super(children, HobbyToken.BINARY);
    }

    public AstNode left() {
        return child(0);
    }

    public String operator() {
        return ((AstLeaf) child(1)).token().getText();
    }

    public AstNode right() {
        return child(2);
    }

    @Override
    public String compile(CodeLine line, int th, int nx) {
        String op = operator();
        if (ScriptParser.ASSIGN_TOKEN.equals(op)) {
            AstNode left = left();

            if (left instanceof IdLiteral) {
                line.addCode(left.compile(line, th, nx) +
                        "=" + right().compile(line, th, nx));
            }
        }
        return null;
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.binaryEval(env, this);
    }
}
