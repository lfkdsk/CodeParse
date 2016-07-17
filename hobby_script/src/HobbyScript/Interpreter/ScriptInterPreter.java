package HobbyScript.Interpreter;

import HobbyScript.ApplicationTest.CodeDialog;
import HobbyScript.Ast.AstNode;
import HobbyScript.Ast.NullStmt;
import HobbyScript.Eval.BasicEnvironment;
import HobbyScript.Eval.EnvironmentCallBack;
import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Parser.ScriptParser;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.logger.Logger;

/**
 * Script解释器
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class ScriptInterpreter {
    public static void run(ScriptParser parser, EnvironmentCallBack env) throws ParseException {
        HobbyLexer lexer = new HobbyLexer(new CodeDialog());

        Logger.init("ScriptInterpreter");

        while (lexer.peek(0) != HobbyToken.EOF) {
            AstNode node = parser.parse(lexer);

            if (!(node instanceof NullStmt)) {
                Object r = node.eval(env);
                Logger.v(" => " + r);
//                PrintUtils.printAstTreeGraph(node);
            }
        }
    }

    public static void main(String[] args) throws ParseException {
        run(new ScriptParser(), new BasicEnvironment());
    }
}
