package HobbyScript.Ast;

import HobbyScript.Compile.CodeLine;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.FunctionEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * 函数定义
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class FuncStmt extends AstList {
    public FuncStmt(List<AstNode> children) {
        super(children, HobbyToken.FUNCTION);
    }

    public String name() {
        return ((AstLeaf) child(0)).token().getText();
    }

    public ParameterList parameters() {
        return (ParameterList) child(1);
    }

    public BlockStmnt body() {
        return (BlockStmnt) child(2);
    }

    public String toString() {
        return "(func " + name() + " " + parameters() + " "
                + body() + " )";
    }

    @Override
    public String compile(CodeLine line, int th, int nx) {

        CodeLine.FunctionCode code = line.addFunction(name(), parameters().childCount());

        line.startCompileFunction(code);
        int begin = line.newLine();
        int end = line.newLine();
        line.addSpecCode("",begin);

        body().compile(line, begin, end);

        line.addSpecCode("", end);
        line.stopCompileFunction();

        return null;
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return FunctionEval.functionEval(env, this);
    }
}
