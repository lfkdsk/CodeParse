package symbols;

import inter.ID;
import lexer.Token;

import java.util.Hashtable;

/**
 * Env把词法单元映射ID
 * 一个代码块内的符号表
 * {
 * // env2
 * int v;
 * {
 * // env1
 * int v;
 * }
 * }
 * Token --> ID
 * Created by liufengkai on 16/3/16.
 */
public class Env {

    private Hashtable<Token, ID> table;

    protected Env prev;

    public Env(Env prev) {
        this.table = new Hashtable<>();
        this.prev = prev;

//        if (prev != null)
//            System.out.println("table " + table.toString()
//                    + " prev " + prev.toString());
    }

    public void put(Token w, ID i) {
        table.put(w, i);
    }

    public ID get(Token w) {
        for (Env e = this; e != null; e = e.prev) {
            ID found = (ID) e.table.get(w);
            if (found != null) return found;
        }
        return null;
    }
}
