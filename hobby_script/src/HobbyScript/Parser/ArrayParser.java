package HobbyScript.Parser;

import HobbyScript.Ast.ArrayIndex;
import HobbyScript.Literal.ArrayLiteral;

/**
 * 支持数组
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/23.
 */
public class ArrayParser extends ClassParser {

    ///////////////////////////////////////////////////////////////////////////
    // element = expr { , expr } 数组元素
    ///////////////////////////////////////////////////////////////////////////
    BnfParser element = BnfParser.rule(ArrayLiteral.class)
            .ast(expr).repeat(BnfParser.rule().sep(COMMA).ast(expr));

    public ArrayParser() {
        reserved.add(LM_TOKEN);
        reserved.add(RM_TOKEN);

        // primary = [ [ element ] ];
        primary.insertChoice(BnfParser.rule().sep(LM_TOKEN).maybe(element)
                .sep(RM_TOKEN));

        // postfix = [ expr ];
        postfix.insertChoice(BnfParser.rule(ArrayIndex.class)
                .sep(LM_TOKEN).ast(expr).sep(RM_TOKEN));
    }
}
