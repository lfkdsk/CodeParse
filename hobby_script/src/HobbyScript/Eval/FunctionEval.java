package HobbyScript.Eval;

import HobbyScript.Ast.*;
import HobbyScript.Exception.HobbyException;
import HobbyScript.Literal.Function;

/**
 * 添加函数方法之后
 * Created by liufengkai on 16/7/16.
 */
public class FunctionEval {

    ///////////////////////////////////////////////////////////////////////////
    // 函数定义
    ///////////////////////////////////////////////////////////////////////////

    /**
     * 创建一个新的函数作用域
     *
     * @param env  环境作用域
     * @param stmt 函数定义
     * @return 函数名
     */
    public static String functionEval(EnvironmentCallBack env, FuncStmt stmt) {

        ((LocalEnvironment) env).putLocal(stmt.name(), new Function(
                stmt.parameters(), stmt.body(), env
        ));

        return stmt.name();
    }

    ///////////////////////////////////////////////////////////////////////////
    // 子表达式的增添项目
    ///////////////////////////////////////////////////////////////////////////

    public static Object operand(PrimaryExpr expr) {
        return expr.child(0);
    }

    public static Postfix postfix(PrimaryExpr expr, int nest) {
        return (Postfix) expr.child(expr.childCount() - nest - 1);
    }

    public static boolean hasPostfix(PrimaryExpr expr, int nest) {
        return expr.childCount() - nest > 1;
    }

    public static Object evalSubExpr(EnvironmentCallBack env,
                                     PrimaryExpr expr,
                                     int nest) {
        if (hasPostfix(expr, nest)) {
            Object target = evalSubExpr(env, expr, nest + 1);
            return postfix(expr, nest).eval(env, target);
        } else {
            return ((AstNode) operand(expr)).eval(env);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Arguments 参数
    ///////////////////////////////////////////////////////////////////////////

    public static Object argumentsEval(EnvironmentCallBack parentEnv,
                                       Arguments args,
                                       Object value) {
        if (!(value instanceof Function)) {
            throw new HobbyException(" wrong function ", args);
        }

        Function function = (Function) value;

        ParameterList parameters = function.parameters();

        // 判断参数数量
        if (args.size() < parameters.size()) {
            throw new HobbyException("args less than define", args);
        } else if (args.size() > parameters.size()) {
            throw new HobbyException("args more than define", args);
        }

        LocalEnvironment newEnv = (LocalEnvironment) function.makeNewEnv();
        if (parentEnv != function.getEnv()) {
            newEnv.setParent(parentEnv);
        } else {
            newEnv.setParent(function.getEnv());
            parentEnv = function.getEnv();
        }


        int num = 0;

        for (AstNode node : args) {
            parameters.eval(newEnv, num++, node.eval(parentEnv));
        }

        return function.body().eval(newEnv);
    }

    ///////////////////////////////////////////////////////////////////////////
    // 闭包函数
    ///////////////////////////////////////////////////////////////////////////

    public static Object closureEval(Closure closure,
                                     EnvironmentCallBack env) {

        return new Function(closure.parameters(),
                closure.body(), env);
    }
}
