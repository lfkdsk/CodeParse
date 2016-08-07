package HobbyScript.Ast;

import HobbyScript.Compile.CodeLine;
import HobbyScript.Compile.Temp;
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
        String leftCom = left().compile(line, th, nx);
        String rightCom = right().compile(line, th, nx);

        if (ScriptParser.ASSIGN_TOKEN.equals(op)) {
            if (left() instanceof IdLiteral) {
                methodForTemp(line, leftCom, rightCom);
            }
        } else {
            Temp tempCode = new Temp();

            if (exp(leftCom)) {
                String temp = leftCom.split("=")[0].replace(" ", "");
                line.addCode(leftCom);
                line.addCode(tempCode + "=" + temp + op + rightCom);
                return tempCode.toString();
            } else if (exp(rightCom)) {
                String temp = rightCom.split("=")[0].replace(" ", "");
                line.addCode(rightCom);
                line.addCode(tempCode + "=" + leftCom + op + temp);
                return tempCode.toString();
            }

            return tempCode.toString() + "=" + leftCom
                    + operator() + rightCom;
        }

        return null;
    }

    private void methodForTemp(CodeLine line, String leftCom, String rightCom) {
        if (exp(rightCom)) {
            String temp = rightCom.split("=")[0].replace(" ", "");

            line.addCode(rightCom);
            line.addCode(leftCom + "=" + temp);
        } else {
            line.addCode(leftCom + "=" + rightCom);
        }
    }

    private boolean exp(String exp) {
        return exp.contains("=");
    }


    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.binaryEval(env, this);
    }
}
