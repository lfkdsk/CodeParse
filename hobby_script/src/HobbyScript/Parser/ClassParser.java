package HobbyScript.Parser;

import HobbyScript.Ast.ClassBody;
import HobbyScript.Ast.ClassStmt;
import HobbyScript.Ast.Dot;
import HobbyScript.Token.HobbyToken;

/**
 * Created by liufengkai on 16/7/21.
 */
public class ClassParser extends ClosureParser {
    BnfParser classLocal = BnfParser.rule().sep("local")
            .or(simple, def);

    BnfParser classOpen = BnfParser.rule().or(simple, def);

    BnfParser member = BnfParser.rule().or(classLocal, classOpen);

    BnfParser classBody = BnfParser.rule(ClassBody.class)
            .sep(LC_TOKEN).option(member)
            .repeat(BnfParser.rule().sep(SEMICOLON_TOKEN, HobbyToken.EOL).option(member))
            .sep(RC_TOKEN);

    BnfParser classDefine = BnfParser.rule(ClassStmt.class)
            .sep(CLASS_TOKEN).identifier(reserved)
            .option(BnfParser.rule().sep(EXTEND_TOKEN).identifier(reserved))
            .ast(classBody);

    public ClassParser() {
        reserved.add(CLASS_TOKEN);
        reserved.add(EXTEND_TOKEN);
        reserved.add(DOT_TOKEN);

        postfix.insertChoice(BnfParser.rule(Dot.class).sep(DOT_TOKEN).identifier(reserved));

        program.insertChoice(classDefine);
    }
}
