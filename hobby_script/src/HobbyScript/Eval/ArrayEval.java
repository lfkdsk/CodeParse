package HobbyScript.Eval;

import HobbyScript.Ast.AstNode;
import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Literal.ArrayLiteral;

/**
 * 新增数组功能
 * Created by liufengkai on 16/7/23.
 */
public class ArrayEval {

    ///////////////////////////////////////////////////////////////////////////
    // 数组初始化
    ///////////////////////////////////////////////////////////////////////////
    public static Object arrayLiteralEval(EnvironmentCallBack env,
                                          ArrayLiteral array) {
        int size = array.childCount();

        Object[] objects = new Object[size];

        int i = 0;
        for (AstNode node : array) {
            objects[i++] = node.eval(env);
        }

        return objects;
    }
}
