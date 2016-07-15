package HobbyScript.Parser;

import HobbyScript.Ast.FuncStmt;
import HobbyScript.Ast.ParameterList;
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

    BnfParser args = BnfParser.rule()
}
