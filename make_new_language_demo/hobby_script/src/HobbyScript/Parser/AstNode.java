package HobbyScript.Parser;

import java.util.Iterator;
import java.util.Objects;
import java.util.function.Consumer;

/**
 * AST TREE NODE 抽象语法树的通用接口
 * Created by liufengkai on 16/7/11.
 */
public abstract class AstNode implements Iterable<AstNode> {
    /**
     * 获取指定子节点
     *
     * @param index 索引
     * @return 子节点
     */
    public abstract AstNode child(int index);

    /**
     * 返回子节点迭代器
     *
     * @return 迭代器
     */
    public abstract Iterator<AstNode> children();

    /**
     * 子节点数目
     *
     * @return count
     */
    public abstract int childCount();

    /**
     * 位置描述
     *
     * @return location
     */
    public abstract String location();

    @Override
    public Iterator<AstNode> iterator() {
        return children();
    }

    @Override
    public void forEach(Consumer<? super AstNode> action) {
        Objects.requireNonNull(action);
        for (AstNode t : this) {
            action.accept(t);
        }
    }
}
