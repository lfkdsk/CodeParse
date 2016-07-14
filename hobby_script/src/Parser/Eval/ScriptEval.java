package Parser.Eval;

import Parser.Ast.AstLeaf;
import Parser.Ast.AstNode;
import Parser.Ast.BinaryExpr;
import Parser.Ast.NegativeExpr;
import Parser.Exception.HobbyException;
import Parser.Literal.IdLiteral;
import Parser.Literal.NumberLiteral;
import Parser.Literal.StringLiteral;
import Parser.Parser.ScriptParser;
import Parser.Token.HobbyToken;
import Parser.Token.IdToken;
import Parser.Token.NumberToken;

/**
 * ScriptEval 表达式求值
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/14.
 */
public class ScriptEval {

    ///////////////////////////////////////////////////////////////////////////
    // 处理基础类型节点
    ///////////////////////////////////////////////////////////////////////////

    /**
     * 字符串求值
     *
     * @param literal stringEval
     * @return 字符串
     */
    public static Object StringEval(StringLiteral literal) {
        return literal.value();
    }

    /**
     * 数字类型求值
     *
     * @param literal NumberEval
     * @return int / double
     */
    public static Object NumberEval(NumberLiteral literal) {
        HobbyToken token = literal.token();
        if (isNum(token.getTag(), token)) {
            return ((NumberToken) token).getNumber().intValue();
        } else if (isFloat(token.getTag(), token)) {
            return ((NumberToken) token).getNumber().doubleValue();
        }
        throw new HobbyException("the number is no legal type: ", literal);
    }

    /**
     * Id
     *
     * @param env     字符表
     * @param literal id
     * @return id-value
     */
    public static Object IdEval(EnvironmentCallBack env, IdLiteral literal) {
        Object value = env.get(literal.name());

        if (value == null) {
            throw new HobbyException("undefined name: ", literal);
        } else {
            return value;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // 取反节点
    ///////////////////////////////////////////////////////////////////////////

    /**
     * 求负数
     *
     * @param expr 求负节点
     * @return 负数
     */
    public static Object negativeEval(EnvironmentCallBack env, NegativeExpr expr) {
        Object value = expr.operand().eval(env);

        HobbyToken token = ((NumberLiteral) expr.operand()).token();

        if (isNum(token.getTag(), token) && value instanceof Integer) {
            return -(Integer) value;
        } else if (isFloat(token.getTag(), token) && value instanceof Double) {
            return -(Double) value;
        }

        throw new HobbyException("bad type for -", expr);
    }

    ///////////////////////////////////////////////////////////////////////////
    // 布尔表达式节点
    ///////////////////////////////////////////////////////////////////////////

    /**
     * 布尔表达式
     *
     * @param env  字符表
     * @param expr binary
     * @return 返回值
     */
    public static Object binaryEval(EnvironmentCallBack env, BinaryExpr expr) {
        String op = expr.operator();

        if (ScriptParser.ASSIGN_TOKEN.equals(op)) {
            Object right = expr.right().eval(env);
            return computeAssign(env, expr, right);
        } else {
            Object left = expr.left().eval(env);
            Object right = expr.right().eval(env);

        }

    }

    /**
     * 处理赋值语句
     *
     * @param env   字符表
     * @param expr  布尔表达式
     * @param value 右值
     * @return 右值
     */
    private static Object computeAssign(EnvironmentCallBack env,
                                        BinaryExpr expr,
                                        Object value) {
        AstLeaf node = (AstLeaf) expr.left();
        int tag = node.token().getTag();
        if (tag == HobbyToken.ID) {
            // 重设值
            env.put(((IdLiteral) node).name(), value);
            return value;
        } else {
            throw new HobbyException("bad assign ", expr);
        }
    }

    private static Object computeOp(Object left, Object right,
                                    String op, BinaryExpr expr) {
        HobbyToken leftToken = ((AstLeaf) expr.left()).token();

        HobbyToken rightToken = ((AstLeaf) expr.right()).token();
        // 判断都是数值
        if (isNumber(leftToken.getTag(), leftToken)
                && isNum(rightToken.getTag(), rightToken)) {

        }
        throw new HobbyException("bad type when eval with op", expr);
    }

    private static boolean isNumber(int tag, HobbyToken token) {
        return (tag == HobbyToken.NUM || tag == HobbyToken.REAL)
                && token.isNumber();
    }

    private static boolean isNum(int tag, HobbyToken token) {
        return tag == HobbyToken.NUM && token.isNumber();
    }


    private static boolean isFloat(int tag, HobbyToken token) {
        return tag == HobbyToken.REAL && token.isNumber();
    }
}
