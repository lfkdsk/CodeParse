package inter;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Break extends Stmt {
    Stmt stmt;

    public Break() {
        if (Stmt.Enclosing == Stmt.Null) error("unenclosed break");
        stmt = Stmt.Enclosing;
    }

    @Override
    public void gen(int th, int nx) {
        emit("goto L" + stmt.after);
    }
}
