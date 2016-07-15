package HobbyScript.Ast;

import java.util.List;

/**
 * 后缀 id() 处理调用
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class Postfix extends AstList {
    public Postfix(List<AstNode> children) {
        super(children);
    }
}
