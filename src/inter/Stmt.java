package inter;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Stmt extends Node {
    public Stmt() {
    }

    public static Stmt Null = new Stmt();

    // 前后两条语句标号
    public void gen(int th, int nx) {
    }

    // 下一条语句标号
    int after = 0;

    // break
    public static Stmt Enclosing = Stmt.Null;
}
