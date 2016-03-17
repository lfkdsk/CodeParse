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

    public int newlabel() {
        return ++labels;
    }

    public void emitlabel(int t) {
        System.out.print("L" + t + ":");
    }

    public void emit(String s) {
        System.out.println("\t" + s);
    }
}
