package HobbyScript.Parser;

import HobbyScript.ApplicationTest.CodeDialog;
import HobbyScript.Ast.AstNode;
import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.StaticType.Ast.TypeLiteral;
import HobbyScript.StaticType.Ast.VarStmt;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.logger.Logger;

/**
 * 静态类型版本
 * 这个当然还需要对应的Eval版本支持
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/16.
 */
public class StaticTypeParser extends FunctionParser {
    BnfParser type = BnfParser.rule().type(TypeLiteral.class);

    BnfParser variable = BnfParser.rule(VarStmt.class)
            .ast(type).identifier(reserved).sep("=").ast(expr);

    public StaticTypeParser() {
        param.reset().ast(type).identifier(reserved);

        // 使用这个记得去修改相应
        def.reset().sep(FUNCTION_TOKEN).ast(type).identifier(reserved)
                .ast(paramList).ast(block);

        statement.insertChoice(variable);
    }

    public static void main(String[] args) throws ParseException {
        HobbyLexer lexer = new HobbyLexer(new CodeDialog());

        Logger.init("FunctionParser");

        ScriptParser parser = new StaticTypeParser();


        while (lexer.peek(0) != HobbyToken.EOF) {
            AstNode node = parser.parse(lexer);

//            if (!(node instanceof NullStmt)){
//                PrintUtils.printAstTreeGraph(node);
//            }

            Logger.v(" => " + node.toString() + "  ");
        }
    }
}
