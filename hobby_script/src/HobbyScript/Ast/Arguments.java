package HobbyScript.Ast;

import java.util.List;

/**
 * 调用参数段
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class Arguments extends Postfix {
    public Arguments(List<AstNode> children) {
        super(children);
    }

    public int size() {
        return childCount();
    }
}
