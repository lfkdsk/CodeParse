package HobbyScript.Compile;

import HobbyScript.ApplicationTest.CodeDialog;
import HobbyScript.Ast.AstNode;
import HobbyScript.Ast.NullStmt;
import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Parser.ArrayParser;
import HobbyScript.Parser.ScriptParser;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Utils.logger.Logger;

import java.io.IOException;

/**
 * Created by liufengkai on 16/8/7.
 */
public class ScriptCompile {


    public static void emitjumps(CodeLine line,
                                 String tests,
                                 int t, int f,
                                 int codeLine) {
        if (t != 0 && f != 0) {
            line.addCode("if " + tests + " goto L" + t, codeLine);
            line.addCode("goto L" + f);
        } else if (t != 0)
            line.addCode("if " + tests + " goto L" + t, codeLine);
        else if (f != 0)
            line.addCode("iffalse " + tests + " goto L" + f, codeLine);
    }


    private static void compile() throws ParseException {
        HobbyLexer lexer = new HobbyLexer(new CodeDialog());

        Logger.init("ScriptParser");

        ScriptParser parser = new ArrayParser();


        while (lexer.peek(0) != HobbyToken.EOF) {
            CodeLine line = new CodeLine();

            int begin = line.newLine();
            int end = line.newLine();

            line.addPrevCode(begin);

            AstNode node = parser.parse(lexer);

            if (!(node instanceof NullStmt)) {
                node.compile(line, begin, end);

                line.addSpecCode("", end);

                line.printList();

                try {
                    line.writeToFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
//            Logger.v(" => " + node.toString() + "  ");
        }


    }

    public static void main(String[] args) throws ParseException {
        compile();
    }
}
