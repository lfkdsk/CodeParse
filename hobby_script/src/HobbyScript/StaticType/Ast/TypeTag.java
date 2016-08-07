package HobbyScript.StaticType.Ast;

import HobbyScript.Ast.AstLeaf;
import HobbyScript.Ast.AstList;
import HobbyScript.Ast.AstNode;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * Created by liufengkai on 16/8/8.
 */
public class TypeTag extends AstList {

    public TypeTag(List<AstNode> children) {
        super(children, HobbyToken.TYPE);
    }

    public static final String INT = "Int";

    public static final String FLOAT = "Float";

    public static final String STRING = "String";

    public enum VarTypeTag {

        INT(HobbyToken.INT),
        FLOAT(HobbyToken.FLOAT),
        STRING(HobbyToken.STRING);

        int tag;

        VarTypeTag(int tag) {
            this.tag = tag;
        }
    }

    public String type() {
        return ((AstLeaf) child(0)).token().getText();
    }
}
