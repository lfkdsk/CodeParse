package symbols;

import inter.ID;
import lexer.Token;

import java.util.Hashtable;

/**
 * Env把词法单元映射ID
 * Created by liufengkai on 16/3/16.
 */
public class Env {

    private Hashtable<Token, ID> table;

    protected Env prev;

    public Env(Env prev) {
        this.table = new Hashtable<>();
        this.prev = prev;
    }

    public void put(Token w, ID i) {
        table.put(w, i);
    }

    public ID get(Token w) {
        return table.get(w) != null ? table.get(w) : null;
    }
}
