package inter;

import lexer.Lexer;

/**
 * 抽象语法树的节点抽象
 * Created by liufengkai on 16/3/16.
 */
public class Node {
    int lexline = 0;

    public Node() {
        lexline = Lexer.line;
    }

    void error(String error) {
        throw new Error("near line " + lexline + ":" + error);
    }

    static int labels = 0;

    /**
     * 新建行
     * @return 当前行号rtry
     */
    public int newLabel() {
        return ++labels;
    }

    /**
     * 打印label
     * @param t
     */
    public void emitLabel(int t) {
        System.out.print("L" + t + ":");
    }

    /**
     * 生成标号
     * @param s
     */
    public void emit(String s) {
        System.out.println("\t" + s);
    }
}
