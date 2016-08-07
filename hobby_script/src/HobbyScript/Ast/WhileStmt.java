package HobbyScript.Ast;

import HobbyScript.Compile.CodeLine;
import HobbyScript.Compile.ScriptCompile;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * While控制块
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class WhileStmt extends AstList {
    public WhileStmt(List<AstNode> children) {
        super(children, HobbyToken.WHILE);
    }

    public AstNode condition() {
        return child(0);
    }

    public AstNode body() {
        return child(1);
    }

    public String toString() {
        return "(while " + condition() + " " + body() + ")";
    }

    @Override
    public String compile(CodeLine line, int start, int end) {
        // 保存状态
        saveList = EnClosingList;
        EnClosingList = this;

        // 保存退出点
        afterPoint = end;                 // 保存用于跳出的地址
        ScriptCompile.emitjumps(line, condition().toString(), 0, end, -1);

        int label = line.newLine();
        line.addPrevCode(label);

        body().compile(line, label, start);
        line.addCode("goto L" + start);        // 打印跳转

        // 恢复状态
        EnClosingList = saveList;
        return null;
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.whileEval(env, new LocalEnvironment(), this);
    }
}
