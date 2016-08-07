package HobbyScript.StaticType.Ast;

import HobbyScript.Ast.AstLeaf;
import HobbyScript.Ast.AstList;
import HobbyScript.Ast.AstNode;
import HobbyScript.Token.HobbyToken;

import java.util.List;

/**
 * Created by liufengkai on 16/8/8.
 */
public class VarStmt extends AstList {


    public TypeTag.VarTypeTag convert(String tag) {
        switch (tag) {
            case TypeTag.FLOAT:
                return TypeTag.VarTypeTag.FLOAT;
            case TypeTag.INT:
                return TypeTag.VarTypeTag.INT;
            case TypeTag.STRING:
                return TypeTag.VarTypeTag.STRING;
            default:
                return null;
        }
    }

    public VarStmt(List<AstNode> children) {
        super(children, HobbyToken.VAR);
    }

    public String name() {
        return ((AstLeaf) child(1)).token().getText();
    }

    public TypeTag.VarTypeTag type() {
        return convert(child(0).toString());
    }

    public AstNode initializer() {
        return child(2);
    }

    @Override
    public String toString() {
        return "(var " + type().tag + " " + name() + " " + initializer().toString() + ")";
    }
}
