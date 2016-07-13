package HobbyScript.Parser;

import HobbyScript.ApplicationTest.CodeDialog;
import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Literal.IdLiteral;
import HobbyScript.Literal.NumberLiteral;
import HobbyScript.Literal.StringLiteral;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.BnfParser;
import HobbyScript.Utils.logger.Logger;

import java.util.HashSet;

/**
 * 构建语法树
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class ScriptParser {
    /**
     * 关键字
     */
    HashSet<String> reserved = new HashSet<>();

    ///////////////////////////////////////////////////////////////////////////
    // 书写 BNF 范式
    ///////////////////////////////////////////////////////////////////////////

    BnfParser.Operators operators = new BnfParser.Operators();

    BnfParser expr0 = BnfParser.rule();

    ///////////////////////////////////////////////////////////////////////////
    // primary = ( expr ) | number | id | string
    ///////////////////////////////////////////////////////////////////////////

    BnfParser primary = BnfParser.rule(PrimaryExpr.class)
            .or(BnfParser.rule().sep("(").ast(expr0).sep(")"),
                    BnfParser.rule().number(NumberLiteral.class),
                    BnfParser.rule().identifier(IdLiteral.class, reserved),
                    BnfParser.rule().string(StringLiteral.class)
            );

    ///////////////////////////////////////////////////////////////////////////
    // factor = primary | - primary
    ///////////////////////////////////////////////////////////////////////////

    BnfParser factor = BnfParser.rule()
            .or(BnfParser.rule(NegativeExpr.class).sep("-").ast(primary), primary);

    ///////////////////////////////////////////////////////////////////////////
    // expr = factor { OP factor }
    ///////////////////////////////////////////////////////////////////////////

    BnfParser expr = expr0.expression(BinaryExpr.class, factor, operators);

    BnfParser statement0 = BnfParser.rule();

    ///////////////////////////////////////////////////////////////////////////
    // block = { statement; * }
    ///////////////////////////////////////////////////////////////////////////

    BnfParser block = BnfParser.rule(BlockStmnt.class)
            .sep("{").option(statement0)
            .repeat(BnfParser.rule().sep(";", HobbyToken.EOL).option(statement0))
            .sep("}");

    BnfParser simple = BnfParser.rule(PrimaryExpr.class).ast(expr).sep(";");

    ///////////////////////////////////////////////////////////////////////////
    // statement = if (expr) block else block | while (expr) block
    ///////////////////////////////////////////////////////////////////////////

    BnfParser statement = statement0
            .or(BnfParser.rule(IfStmnt.class).sep("if").sep("(")
                            .ast(expr).sep(")").ast(block)
                            .option(BnfParser.rule().sep("else").ast(block)),
                    BnfParser.rule(WhileStmt.class).sep("while").sep("(")
                            .ast(expr).sep(")").ast(block), simple);

    ///////////////////////////////////////////////////////////////////////////
    // program = statement | (; , end of line)
    ///////////////////////////////////////////////////////////////////////////

    BnfParser program = BnfParser.rule().or(statement,
            BnfParser.rule(NullStmt.class).sep(";", HobbyToken.EOL));

    ///////////////////////////////////////////////////////////////////////////
    // 构造添加
    ///////////////////////////////////////////////////////////////////////////

    public ScriptParser() {
        reserved.add(";");
        reserved.add("}");
        reserved.add(")");
        reserved.add(HobbyToken.EOL);

        operators.add("=", 1, BnfParser.Operators.RIGHT);
        operators.add("==", 2, BnfParser.Operators.LEFT);
        operators.add(">", 2, BnfParser.Operators.LEFT);
        operators.add("<", 2, BnfParser.Operators.LEFT);
        operators.add("+", 3, BnfParser.Operators.LEFT);
        operators.add("-", 3, BnfParser.Operators.LEFT);
        operators.add("*", 4, BnfParser.Operators.LEFT);
        operators.add("/", 4, BnfParser.Operators.LEFT);
        operators.add("%", 4, BnfParser.Operators.LEFT);
    }

    public AstNode parse(HobbyLexer lexer) throws ParseException {
        return program.parse(lexer);
    }

    public static void main(String[] args) throws ParseException {
        HobbyLexer lexer = new HobbyLexer(new CodeDialog());

        Logger.init();

        ScriptParser parser = new ScriptParser();

        while (lexer.peek(0) != HobbyToken.EOF) {
            AstNode node = parser.parse(lexer);

            Logger.i(" => " + node.toString() + "  ");
        }
    }
}
