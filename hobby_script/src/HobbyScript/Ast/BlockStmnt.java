package HobbyScript.Ast;

import HobbyScript.Eval.EnvironmentCallBack;
import HobbyScript.Eval.ScriptEval;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * 代码块
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class BlockStmnt extends AstList {

    public BlockStmnt(List<AstNode> children) {
        super(children, HobbyToken.BLOCK);
    }

    @Override
    public Object eval(EnvironmentCallBack env) {
        return ScriptEval.blockEval(env, this);
    }
}
