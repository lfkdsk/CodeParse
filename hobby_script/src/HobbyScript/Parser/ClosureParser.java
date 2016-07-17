package HobbyScript.Parser;

import HobbyScript.Ast.Closure;

/**
 * Created by liufengkai on 16/7/17.
 */
public class ClosureParser extends FunctionParser {

    public ClosureParser() {
        primary.insertChoice(BnfParser.rule(Closure.class)
                .sep(CLOSURE_TOKEN).ast(paramList).ast(block));
    }
}
