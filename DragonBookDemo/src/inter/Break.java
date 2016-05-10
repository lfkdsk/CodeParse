package inter;

/**
 * Break
 * Created by liufengkai on 16/3/17.
 */
public class Break extends Stmt {
    Stmt stmt;

    public Break() {
        if (Stmt.Enclosing == Stmt.Null) error("unenclosed break");
        stmt = Stmt.Enclosing;
    }

    /**
     * 生成/跳
     *
     * @param th
     * @param nx
     */
    @Override
    public void gen(int th, int nx) {
        emit("goto L" + stmt.after);
    }
}
