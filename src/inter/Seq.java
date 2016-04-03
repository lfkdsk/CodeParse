package inter;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Seq extends Stmt {
    Stmt stmt1;
    Stmt stmt2;

    public Seq(Stmt stmt1, Stmt stmt2) {
        this.stmt1 = stmt1;
        this.stmt2 = stmt2;
    }

    @Override
    public void gen(int th, int nx) {
        if (stmt1 == Stmt.Null) stmt2.gen(th, nx);
        else if (stmt2 == Stmt.Null) stmt1.gen(th, nx);
        else {
            int label = newLabel();
            stmt1.gen(th, label);
            emitLabel(label);
            stmt2.gen(label, nx);
        }
    }
}
