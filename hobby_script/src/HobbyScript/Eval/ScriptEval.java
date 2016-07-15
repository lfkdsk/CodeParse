package HobbyScript.Eval;

import HobbyScript.Ast.*;
import HobbyScript.Exception.HobbyException;
import HobbyScript.Literal.IdLiteral;
import HobbyScript.Literal.NumberLiteral;
import HobbyScript.Literal.StringLiteral;
import HobbyScript.Parser.ScriptParser;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Token.NumberToken;

import java.util.Iterator;

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
        if (isNum(((NumberToken) token).getNumber())) {
            return ((NumberToken) token).getNumber().intValue();
        } else if (isFloat(((NumberToken) token).getNumber())) {
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

        if (isNum(value)) {
            return -(Integer) value;
        } else if (isFloat(value)) {
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
            return computeOp(left, right, op, expr);
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

        // 判断都是数值
        if (isNumber(left) && isNumber(right)) {
            Object temp = computeNumber(left, right, op, expr);
            if (isNum(left) && isNum(right) && isNumber(temp)) {
                return ((Number) temp).intValue();
            } else {
                return temp;
            }
        }

        if (op.equals(ScriptParser.ADD) && (left instanceof String)) {
            return String.valueOf(left) + String.valueOf(right);
        }

        if (op.equals(ScriptParser.EQ_TOKEN)) {
            if (left == null) {
                return right == null;
            } else {
                return left.equals(right);
            }
        }

        throw new HobbyException("bad type when eval with op", expr);
    }

    private static Object computeNumber(Object left, Object right,
                                        String op, BinaryExpr expr) {
        // 此时 left 至少肯定是数字了
        switch (op) {
            case ScriptParser.ADD:
                return (computeIntValue(left) + computeFloatValue(left)) +
                        (computeIntValue(right) + computeFloatValue(right));
            case ScriptParser.SUB:
                return (computeIntValue(left) + computeFloatValue(left)) -
                        (computeIntValue(right) + computeFloatValue(right));
            case ScriptParser.MUL:
                return (computeIntValue(left) + computeFloatValue(left)) *
                        (computeIntValue(right) + computeFloatValue(right));
            case ScriptParser.DIV:
                return (computeIntValue(left) + computeFloatValue(left)) /
                        (computeIntValue(right) + computeFloatValue(right));
            case ScriptParser.MOD:
                return (computeIntValue(left) + computeFloatValue(left)) %
                        (computeIntValue(right) + computeFloatValue(right));
            case ScriptParser.EQ_TOKEN:
                return (computeIntValue(left) + computeFloatValue(left)) ==
                        (computeIntValue(right) + computeFloatValue(right))
                        ? Boolean.TRUE : Boolean.FALSE;
            case ScriptParser.GE_TOKEN:
                return (computeIntValue(left) + computeFloatValue(left)) >
                        (computeIntValue(right) + computeFloatValue(right))
                        ? Boolean.TRUE : Boolean.FALSE;
            case ScriptParser.GT_TOKEN:
                return (computeIntValue(left) + computeFloatValue(left)) <
                        (computeIntValue(right) + computeFloatValue(right))
                        ? Boolean.TRUE : Boolean.FALSE;
            default:
                break;
        }

        throw new HobbyException("bad operator", expr);
    }

    private static int computeIntValue(Object v) {
        return v instanceof Integer ? (Integer) v : 0;
    }

    private static double computeFloatValue(Object v) {
        return v instanceof Double ? (Double) v : (int) 0;
    }

    private static boolean isNumber(Object v) {
        return (v instanceof Integer || v instanceof Double);
    }

    private static boolean isString(int tag) {
        return tag == HobbyToken.STRING;
    }

    private static boolean isNum(Object v) {
        return v instanceof Integer;
    }

    private static boolean isFloat(Object v) {
        return v instanceof Double;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Block 节点
    ///////////////////////////////////////////////////////////////////////////

    public static Object blockEval(EnvironmentCallBack env, BlockStmnt expr) {
        Object result = 0;
        Iterator<AstNode> iterator = expr.iterator();

        while (iterator.hasNext()) {
            AstNode node = iterator.next();
            if (!(node instanceof NullStmt)) {
                result = node.eval(env);
            }
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // If 节点
    ///////////////////////////////////////////////////////////////////////////

    public static Object ifEval(EnvironmentCallBack env, IfStmnt ifStmnt) {
        Object c = ifStmnt.condition().eval(env);

        if (c instanceof Boolean && ((Boolean) c).booleanValue() == Boolean.TRUE) {
            return ifStmnt.thenBlock().eval(env);
        } else if (c instanceof Integer && (Integer) c > 0) {
            return ifStmnt.thenBlock().eval(env);
        } else if (c instanceof Double && (Double) c > 0) {
            return ifStmnt.thenBlock().eval(env);
        } else {
            AstNode node = ifStmnt.elseBlock();
            if (node == null) {
                return 0;
            } else {
                return node.eval(env);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // While节点
    ///////////////////////////////////////////////////////////////////////////

    public static Object whileEval(EnvironmentCallBack env,
                                   WhileStmt whileStmt) {
        Object result = 0;

        for (; ; ) {
            Object c = whileStmt.condition().eval(env);

            if (c instanceof Boolean &&
                    ((Boolean) c).booleanValue() == Boolean.TRUE) {
                result = whileStmt.body().eval(env);
            } else if (c instanceof Integer && (Integer) c > 0) {
                result = whileStmt.body().eval(env);
            } else if (c instanceof Double && (Double) c > 0) {
                result = whileStmt.body().eval(env);
            } else {
                return result;
            }
        }
    }
}
