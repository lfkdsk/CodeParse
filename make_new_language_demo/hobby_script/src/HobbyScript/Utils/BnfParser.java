package HobbyScript.Utils;

import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Parser.AstLeaf;
import HobbyScript.Parser.AstNode;
import com.sun.org.apache.xpath.internal.operations.Or;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

/**
 * 巴克斯范式的parser
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class BnfParser {

    protected static abstract class Element {
        protected abstract void parse(HobbyLexer lexer, List<AstNode> nodes)
                throws ParseException;

        protected abstract boolean match(HobbyLexer lexer) throws ParseException;
    }

    protected static class Tree extends Element {
        protected BnfParser parser;

        public Tree(BnfParser parser) {
            this.parser = parser;
        }

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            nodes.add(parser.parse(lexer));
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return parser.match(lexer);
        }
    }

    protected static class OrTree extends Element {
        protected BnfParser[] parsers;

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            BnfParser parser = choose(lexer);
            if (parser == null) {
                throw new ParseException(lexer.peek(0));
            } else {
                nodes.add(parser.parse(lexer));
            }
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return false;
        }

        protected BnfParser choose(HobbyLexer lexer) throws ParseException {
            for (BnfParser parser : parsers) {
                if (parser.match(lexer)) {
                    return parser;
                }
            }
            return null;
        }

        protected void insert(BnfParser parser) {
            BnfParser[] newParsers = new BnfParser[parsers.length + 1];
            newParsers[0] = parser;
            System.arraycopy(parsers, 0, newParsers, 1, parsers.length);
            parsers = newParsers;
        }
    }


    protected static class Repeat extends Element {

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {

        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return false;
        }
    }

    protected static abstract class AToken extends Element {

    }

    protected static class IdToken extends AToken {

    }

    protected static class NumToken extends AToken {

    }

    protected static class StrToken extends AToken {

    }

    protected static class Leaf extends Element {

    }

    protected static class Skip extends Leaf {

    }

    protected static class Precedence {

    }

    public static class Operators extends HashMap<String, Precedence> {

    }

    protected static class Expr extends Element {

    }

    public static final String factoryName = "create";

    protected abstract static class Factory {

    }

    protected List<Element> elements;

    protected Factory factory;

    public BnfParser(Class<? extends AstNode> clazz) {
        reset(clazz);
    }

    protected BnfParser(BnfParser parser) {
        elements = parser.elements;
        factory = parser.factory;
    }

    public AstNode parse(HobbyLexer lexer) throws ParseException {
        ArrayList<AstNode> results = new ArrayList<>();
        for (Element e : elements) {
            e.parse(lexer, results);
        }
        return factory.make(results);
    }


    protected boolean match(HobbyLexer lexer) throws ParseException {
        if (elements.size() == 0) {
            return true;
        } else {
            Element e = elements.get(0);
            return e.match(lexer);
        }
    }

    public static BnfParser rule() {
        return rule(null);
    }

    public static BnfParser rule(Class<? extends AstNode> clazz) {
        return new BnfParser(clazz);
    }

    public BnfParser reset() {
        elements = new ArrayList<>();
        return this;
    }

    public BnfParser reset(Class<? extends AstNode> clazz) {
        elements = new ArrayList<>();
        factory = Factory.getForAstList(clazz);
        return this;
    }

    public BnfParser number() {
        return number(null);
    }

    public BnfParser number(Class<? extends AstNode> clazz) {
        return new BnfParser(clazz);
    }

    public BnfParser identifier(HashSet<String> reserved) {
        return identifier(null, reserved);
    }

    public BnfParser identifier(Class<? extends AstLeaf> clazz,
                                HashSet<String> reserved) {
        elements.add(new IdToken(clazz, reserved));
        return this;
    }

    public BnfParser string() {
        return string(null);
    }

    public BnfParser string(Class<? extends AstLeaf> clazz) {
        elements.add(new StrToken(clazz));
        return this;
    }

    public BnfParser token(String... pat) {
        elements.add(new Skip(pat));
        return this;
    }

    public BnfParser sep(String... pat) {
        elements.add(new Leaf(pat));
        return this;
    }

    public BnfParser ast(BnfParser parser) {
        elements.add(new Skip(parser));
        return this;
    }

    public BnfParser or(BnfParser... parsers) {
        elements.add(new OrTree(parsers));
        return this;
    }

    public BnfParser maybe(BnfParser parser) {
        BnfParser parser1 = new BnfParser(parser);

        parser1.reset();

        elements.add(new OrTree(new BnfParser[]{parser, parser1}));
        return this;
    }

    public BnfParser option(BnfParser parser) {
        elements.add(new Repeat(parser, true));
        return this;
    }

    public BnfParser repeat(BnfParser parser) {
        elements.add(new Repeat(parser, false));
        return this;
    }

    public BnfParser expression(BnfParser subExp, Operators operators) {
        elements.add(new Expr(null, subExp, operators));
        return this;
    }

    public BnfParser expression(Class<? extends AstNode> clazz, BnfParser subExp,
                                Operators operators) {
        elements.add(new Expr(clazz, subExp, operators));
        return this;
    }

    public BnfParser insertChoice(BnfParser parser) {
        Element e = elements.get(0);
        if (e instanceof OrTree) {
            ((OrTree) e).insert(parser);
        } else {
            BnfParser otherWise = new BnfParser(this);
            reset(null);
            or(parser, otherWise);
        }
        return this;
    }
}
