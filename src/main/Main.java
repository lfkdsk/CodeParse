package main;

import lexer.Lexer;
import parser.Parser;

/**
 * Created by liufengkai on 16/3/20.
 */
public class Main {
    public static void main(String[] args) {
        Lexer lexer = new Lexer();

        Parser parser = new Parser(lexer);

        lexer.printAll();

        parser.program();
        System.out.print('\n');
    }
}
