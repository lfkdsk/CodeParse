package HobbyScript.Parser;

import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Literal.IdLiteral;
import HobbyScript.Literal.NumberLiteral;
import HobbyScript.Literal.StringLiteral;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.BnfParser;

import java.util.HashSet;

/**
 * 构建语法树
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class ScriptParser {
    HashSet<String> reserved = new HashSet<>();

    BnfParser.Operators operators = new BnfParser.Operators();

    BnfParser expr0 = BnfParser.rule();

    BnfParser primary = BnfParser.rule(PrimaryExpr.class)
            .or(BnfParser.rule().sep("(").ast(expr0).sep(")"),
                    BnfParser.rule().number(NumberLiteral.class),
                    BnfParser.rule().identifier(IdLiteral.class, reserved),
                    BnfParser.rule().string(StringLiteral.class)
            );

    BnfParser factor = BnfParser.rule()
            .or(BnfParser.rule(NegativeExpr.class).sep("-").ast(primary), primary);

    BnfParser expr = expr0.expression(BinaryExpr.class, factor, operators);

    BnfParser statement0 = BnfParser.rule();

    BnfParser block = BnfParser.rule(BlockStmnt.class)
            .sep("{").option(statement0)
            .repeat(BnfParser.rule().sep(";", HobbyToken.EOL).option(statement0));

    BnfParser simple = BnfParser.rule(PrimaryExpr.class).ast(expr);

    BnfParser statement = statement0
            .or(BnfParser.rule(IfStmnt.class).sep("if").ast(expr).ast(block)
                            .option(BnfParser.rule().sep("else").ast(block)),
                    BnfParser.rule(WhileStmnt.class).sep("while")
                            .ast(expr).ast(block));

    BnfParser program = BnfParser.rule().or(statement,
            BnfParser.rule(NullStmnt.class).sep(";", HobbyToken.EOL));

    ///////////////////////////////////////////////////////////////////////////
    // 构造添加
    ///////////////////////////////////////////////////////////////////////////

    public ScriptParser() {
        reserved.add(";");
        reserved.add("}");
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

    public AstNode parser(HobbyLexer lexer) throws ParseException {
        return program.parse(lexer);
    }
}
