package HobbyScript.Eval;

import HobbyScript.Ast.AstNode;
import HobbyScript.Ast.ClassBody;
import HobbyScript.Ast.ClassStmt;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Literal.ClassInfo;

/**
 * Created by liufengkai on 16/7/21.
 */
public class ClassEval {

    public static String classDefineEval(EnvironmentCallBack env,
                                         ClassStmt stmt) {
        ClassInfo info = new ClassInfo(stmt, env);
        env.put(stmt.name(), info);
        return stmt.name();
    }

    public static Object classBodyEval(EnvironmentCallBack env,
                                       ClassBody body) {
        for (AstNode n : body) {
            n.eval(env);
        }
        return null;
    }
}
