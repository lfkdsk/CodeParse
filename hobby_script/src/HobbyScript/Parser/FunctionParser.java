package HobbyScript.Parser;

import HobbyScript.Ast.Arguments;
import HobbyScript.Ast.FuncStmt;
import HobbyScript.Ast.ParameterList;
import HobbyScript.Ast.PrimaryExpr;
import HobbyScript.Utils.BnfParser;

/**
 * Created by liufengkai on 16/7/15.
 */
public class FunctionParser extends ScriptParser {
    BnfParser param = BnfParser.rule().identifier(reserved);

    BnfParser params = BnfParser.rule(ParameterList.class)
            .ast(param).repeat(BnfParser.rule().sep(COMMA).ast(param));

    BnfParser paramList = BnfParser.rule().sep(LP_TOKEN)
            .maybe(params).sep(RP_TOKEN);

    BnfParser def = BnfParser.rule(FuncStmt.class)
            .sep(FUNCTION_TOKEN).identifier(reserved)
            .ast(paramList).ast(block);

    BnfParser args = BnfParser.rule(Arguments.class)
            .ast(expr).repeat(BnfParser.rule().sep(COMMA).ast(expr));

    BnfParser postfix = BnfParser.rule().sep(LP_TOKEN)
            .maybe(args).sep(RP_TOKEN);


    public FunctionParser() {
        primary.repeat(postfix);

        // 更换 simple 语句
        simple = BnfParser.rule(PrimaryExpr.class).ast(expr).option(args);

        program.insertChoice(def);
    }

}
