package inter;

/**
 * 语句构造
 * Created by liufengkai on 16/3/17.
 */
public class Stmt extends Node {
    public Stmt() {
    }

    public static Stmt Null = new Stmt();

    /**
     * 处理三地址指令生成
     * @param th 标记语句代码开始的地方
     * @param nx 标记这个代码后的第一条语句
     */

    /**
     * {: th
     *     xxxx
     * }
     * : nx
     */

    // 前后两条语句标号
    public void gen(int th, int nx) {
    }

    // 下一条语句标号
    // 子类while和do把他们的标号nx放到after里面去了
    // 所以出现break;的时候就可以直接跳出了
    // 包含continue可以一次类推
    int after = 0;

    // 用于break语句
    public static Stmt Enclosing = Stmt.Null;
}
