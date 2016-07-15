package HobbyScript.Ast;

import java.util.List;

/**
 * 定义参数段
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/15.
 */
public class ParameterList extends AstList {
    public ParameterList(List<AstNode> children) {
        super(children);
    }

    public int size() {
        return childCount();
    }

    /**
     * 获取对应参数的名字
     *
     * @param i index
     * @return name
     */
    public String name(int i) {
        return ((AstLeaf) children.get(i)).token().getText();
    }

}
