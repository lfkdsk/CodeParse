package HobbyScript.Ast;

import HobbyScript.Compile.CodeLine;
import HobbyScript.Compile.ScriptCompile;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * IF 控制块
 * Created by liufengkai on 16/7/12.
 */
public class IfStmnt extends AstList {

    public IfStmnt(List<AstNode> children) {
        super(children, HobbyToken.IF);
    }

    public AstNode condition() {
        return child(0);
    }

    public AstNode thenBlock() {
        return child(1);
    }

    public AstNode elseBlock() {
        return childCount() > 2 ? child(2) : null;
    }

    public String toString() {
        return "(if " + condition() + " " +
                thenBlock() + " else " +
                elseBlock() + " )";
    }

    @Override
    public String compile(CodeLine line, int th, int nx) {

        if (elseBlock() == null || thenBlock().childCount() <= 0) {
            // stmt 标号
            int label = line.newLine();
            // jump
            ScriptCompile.emitjumps(line, condition().toString(), 0, nx, -1);

            line.addPrevCode(label);
            // 为真时控制穿越流,为假时转向nx
            thenBlock().compile(line, label, nx);
        } else {
            int label_1 = line.newLine(); // stmt1
            int label_2 = line.newLine(); // stmt2

            ScriptCompile.emitjumps(line, condition().toString(), 0, label_2, label_1); // 为真时1

            thenBlock().compile(line, label_1, nx);

            line.addCode("goto L" + nx);

            line.addPrevCode(label_2);

            elseBlock().compile(line, label_2, nx);
        }

        return null;
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.ifEval(env, new LocalEnvironment(), this);
    }
}
