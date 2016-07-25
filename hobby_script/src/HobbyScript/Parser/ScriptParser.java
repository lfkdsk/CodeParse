package HobbyScript.Parser;

import HobbyScript.ApplicationTest.CodeDialog;
import HobbyScript.Ast.*;
import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Literal.IdLiteral;
import HobbyScript.Literal.NumberLiteral;
import HobbyScript.Literal.StringLiteral;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.logger.Logger;

import java.util.HashSet;

/**
 * 构建语法树
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class ScriptParser {

    public static final String IF_TOKEN = "if";

    public static final String ELSE_TOKEN = "else";

    public static final String WHILE_TOKEN = "while";

    public static final String FUNCTION_TOKEN = "function";

    public static final String SEMICOLON_TOKEN = ";";

    public static final String LP_TOKEN = "(", RP_TOKEN = ")";

    public static final String LC_TOKEN = "{", RC_TOKEN = "}";

    public static final String ASSIGN_TOKEN = "=";

    public static final String EQ_TOKEN = "==";

    public static final String LOGICAL_AND_TOKEN = "&&";

    public static final String GT_TOKEN = "<", GE_TOKEN = ">";

    public static final String ADD = "+", SUB = "-",
            MUL = "*", DIV = "/", MOD = "%";

    public static final String COMMA = ",";

    public static final String FOR_TOKEN = "for";

    public static final String BREAK_TOKEN = "break";

    public static final String CLOSURE_TOKEN = "closure";

    public static final String CLASS_TOKEN = "class";

    public static final String EXTEND_TOKEN = "extend";

    public static final String DOT_TOKEN = ".",
            INITIAL = "Initial", THIS_POINT = "this",
            SUPER_TOKEN = "super";

    public static final String LM_TOKEN = "[", RM_TOKEN = "]";

    public static final String LS_TOKEN = "<", RS_TOKEN = ">";

    /**
     * 保留关键字
     */
    protected HashSet<String> reserved = new HashSet<>();

    ///////////////////////////////////////////////////////////////////////////
    // 书写 BNF 范式
    ///////////////////////////////////////////////////////////////////////////

    BnfParser.Operators operators = new BnfParser.Operators();

    BnfParser expr0 = BnfParser.rule();


    BnfParser number = BnfParser.rule().number(NumberLiteral.class);

    BnfParser id = BnfParser.rule().identifier(IdLiteral.class, reserved);

    BnfParser string = BnfParser.rule().string(StringLiteral.class);

    ///////////////////////////////////////////////////////////////////////////
    // primary = ( expr ) | number | id | string
    ///////////////////////////////////////////////////////////////////////////

    BnfParser primary = BnfParser.rule(PrimaryExpr.class)
            .or(BnfParser.rule().sep(LP_TOKEN).ast(expr0).sep(RP_TOKEN),
                    number,
                    id,
                    string
            );

    ///////////////////////////////////////////////////////////////////////////
    // factor = primary | - primary
    ///////////////////////////////////////////////////////////////////////////

    BnfParser factor = BnfParser.rule()
            .or(BnfParser.rule(NegativeExpr.class).sep(SUB).ast(primary), primary);

    ///////////////////////////////////////////////////////////////////////////
    // expr = factor { OP factor }
    ///////////////////////////////////////////////////////////////////////////

    BnfParser expr = expr0.expression(BinaryExpr.class, factor, operators);

    BnfParser statement0 = BnfParser.rule();

    ///////////////////////////////////////////////////////////////////////////
    // simple = expr ;
    ///////////////////////////////////////////////////////////////////////////

    BnfParser simple = BnfParser.rule().ast(expr).sep(SEMICOLON_TOKEN);

    ///////////////////////////////////////////////////////////////////////////
    // block = { statement; * }
    ///////////////////////////////////////////////////////////////////////////

    BnfParser block = BnfParser.rule(BlockStmnt.class)
            .sep(LC_TOKEN).option(statement0)
            .repeat(BnfParser.rule().sep(SEMICOLON_TOKEN, HobbyToken.EOL)
                    .option(statement0))
            .sep(RC_TOKEN);

    ///////////////////////////////////////////////////////////////////////////
    // if ()
    ///////////////////////////////////////////////////////////////////////////
    BnfParser ifStatement =
            BnfParser.rule(IfStmnt.class).sep(IF_TOKEN).sep(LP_TOKEN)
                    .ast(expr).sep(RP_TOKEN).ast(block)
                    .option(BnfParser.rule().sep(ELSE_TOKEN).ast(block));

    ///////////////////////////////////////////////////////////////////////////
    // while ( condition ) {  }
    ///////////////////////////////////////////////////////////////////////////

    BnfParser whileStatement =
            BnfParser.rule(WhileStmt.class).sep(WHILE_TOKEN).sep(LP_TOKEN)
                    .ast(expr).sep(RP_TOKEN).ast(block);

    ///////////////////////////////////////////////////////////////////////////
    // for (initial ; condition ; step)
    ///////////////////////////////////////////////////////////////////////////

    BnfParser option = BnfParser.rule(OptionStmt.class).option(expr)
            .repeat(BnfParser.rule().sep(COMMA).option(expr));

    BnfParser forStatement =
            BnfParser.rule(ForStmt.class).sep(FOR_TOKEN)
                    .sep(LP_TOKEN)
                    .or(option, BnfParser.rule(NullStmt.class)).sep(SEMICOLON_TOKEN)
                    .or(expr, BnfParser.rule(NullStmt.class)).sep(SEMICOLON_TOKEN)
                    .or(option, BnfParser.rule(NullStmt.class))
                    .sep(RP_TOKEN).ast(block);

    ///////////////////////////////////////////////////////////////////////////
    // break;
    ///////////////////////////////////////////////////////////////////////////

    BnfParser breakStatement = BnfParser.rule(BreakStmt.class)
            .sep(BREAK_TOKEN);

    ///////////////////////////////////////////////////////////////////////////
    // statement = if (expr) block else block | while (expr) block
    ///////////////////////////////////////////////////////////////////////////

    BnfParser statement = statement0
            .or(ifStatement, whileStatement, forStatement, breakStatement, simple);

    ///////////////////////////////////////////////////////////////////////////
    // program = statement | (; , end of line)
    ///////////////////////////////////////////////////////////////////////////

    BnfParser program = BnfParser.rule().or(statement,
            BnfParser.rule(NullStmt.class).sep(SEMICOLON_TOKEN, HobbyToken.EOL));

    ///////////////////////////////////////////////////////////////////////////
    // 构造添加
    ///////////////////////////////////////////////////////////////////////////

    public ScriptParser() {
        reserved.add(SEMICOLON_TOKEN);
        reserved.add(RC_TOKEN);
        reserved.add(IF_TOKEN);
        reserved.add(WHILE_TOKEN);
        reserved.add(FOR_TOKEN);
        reserved.add(RP_TOKEN);
        reserved.add(BREAK_TOKEN);
        reserved.add(ELSE_TOKEN);
        reserved.add(HobbyToken.EOL);

        operators.add(ASSIGN_TOKEN, 1, BnfParser.Operators.RIGHT);
        operators.add(EQ_TOKEN, 2, BnfParser.Operators.LEFT);
        operators.add(GE_TOKEN, 2, BnfParser.Operators.LEFT);
        operators.add(GT_TOKEN, 2, BnfParser.Operators.LEFT);
        operators.add(ADD, 3, BnfParser.Operators.LEFT);
        operators.add(SUB, 3, BnfParser.Operators.LEFT);
        operators.add(MUL, 4, BnfParser.Operators.LEFT);
        operators.add(DIV, 4, BnfParser.Operators.LEFT);
        operators.add(MOD, 4, BnfParser.Operators.LEFT);
    }

    public AstNode parse(HobbyLexer lexer) throws ParseException {
        return program.parse(lexer);
    }

    public static void main(String[] args) throws ParseException {
        HobbyLexer lexer = new HobbyLexer(new CodeDialog());

        Logger.init("ScriptParser");

        ScriptParser parser = new ScriptParser();


        while (lexer.peek(0) != HobbyToken.EOF) {
            AstNode node = parser.parse(lexer);

//            if (!(node instanceof NullStmt)) {
//                PrintUtils.printAstTreeGraph(node);
//            }

            Logger.v(" => " + node.toString() + "  ");
        }
    }
}
