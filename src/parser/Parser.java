package parser;

import lexer.Lexer;
import lexer.Token;
import symbols.Env;

/**
 * Created by liufengkai on 16/3/17.
 */
public class Parser {
    private Lexer lexer;
    private Token look;
    Env top = null;
    int used = 0;

    public Parser(Lexer lexer) {
        this.lexer = lexer;
        move();
    }

    void move() {
        look = lexer.scan();
    }

    void error(String s) {
        throw new Error("near line " + Lexer.line + ": " + s);
    }

    void match(int t) {
        if (look.tag == t) move();
        else error("syntax error");
    }
}
