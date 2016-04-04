package main;

import lexer.Lexer;
import parser.Parser;

import java.io.IOException;

/**
 *
 * Created by liufengkai on 16/3/20.
 */
public class Main {
    public static void main(String[] args) throws IOException {
        Lexer lexer = new Lexer();

        Parser parser;
        parser = new Parser(lexer);

        parser.program();


        System.out.print('\n');
    }
}
