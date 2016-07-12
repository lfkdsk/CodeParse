package HobbyScript.Utils;

import HobbyScript.Exception.ParseException;
import HobbyScript.Lexer.HobbyLexer;
import HobbyScript.Parser.AstLeaf;
import HobbyScript.Parser.AstList;
import HobbyScript.Parser.AstNode;
import HobbyScript.Token.HobbyToken;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.*;

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

        public OrTree(BnfParser[] parsers) {
            this.parsers = parsers;
        }

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
            return choose(lexer) != null;
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
        protected BnfParser parser;

        protected boolean onlyOne;

        public Repeat(BnfParser parser, boolean onlyOne) {
            this.parser = parser;
            this.onlyOne = onlyOne;
        }

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            while (parser.match(lexer)) {
                AstNode node = parser.parse(lexer);
                if (node.getClass() != AstList.class || node.childCount() > 0) {
                    nodes.add(node);
                }

                if (onlyOne)
                    break;
            }
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return parser.match(lexer);
        }
    }

    protected static abstract class AToken extends Element {
        protected Factory factory;

        public AToken(Class<? extends AstLeaf> clazz) {
            if (clazz == null) {
                clazz = AstLeaf.class;
            }

            factory = Factory.get(clazz, HobbyToken.class);
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return Token_Test(lexer.peek(0));
        }

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            HobbyToken token = lexer.read();

            if (Token_Test(token)) {
                AstNode leaf = factory.make(token);

                nodes.add(leaf);
            } else {
                throw new ParseException(token);
            }
        }

        protected abstract boolean Token_Test(HobbyToken token);
    }

    protected static class IdToken extends AToken {
        HashSet<String> reserved;

        public IdToken(Class<? extends AstLeaf> clazz, HashSet<String> reserved) {
            super(clazz);
            this.reserved = reserved != null ? reserved : new HashSet<>();
        }

        @Override
        protected boolean Token_Test(HobbyToken token) {
            return token.isIdentifier() && !reserved.contains(token.getText());
        }
    }

    protected static class NumToken extends AToken {

        public NumToken(Class<? extends AstLeaf> clazz) {
            super(clazz);
        }

        @Override
        protected boolean Token_Test(HobbyToken token) {
            return token.isNumber();
        }

    }

    protected static class StrToken extends AToken {

        public StrToken(Class<? extends AstLeaf> clazz) {
            super(clazz);
        }

        @Override
        protected boolean Token_Test(HobbyToken token) {
            return token.isString();
        }
    }

    protected static class Leaf extends Element {
        protected String[] tokens;

        protected Leaf(String[] pat) {
            this.tokens = pat;
        }

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            HobbyToken token = lexer.read();

            if (token.isIdentifier()) {
                for (String t : tokens) {
                    if (t.equals(token.getText())) {
                        find(nodes, token);
                        return;
                    }
                }
            }

            if (tokens.length > 0) {
                throw new ParseException(tokens[0] + " expected. ", token);
            } else {
                throw new ParseException(token);
            }
        }

        protected void find(List<AstNode> list, HobbyToken token) {
            list.add(new AstLeaf(token));
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            HobbyToken token = lexer.peek(0);

            if (token.isIdentifier()) {
                for (String t : tokens) {
                    if (t.equals(token.getText())) {
                        return true;
                    }
                }
            }

            return false;
        }
    }

    protected static class Skip extends Leaf {

        protected Skip(String[] pat) {
            super(pat);
        }

        @Override
        protected void find(List<AstNode> list, HobbyToken token) {

        }
    }

    protected static class Precedence {
        int value;
        boolean leftAssoc;

        public Precedence(int value, boolean leftAssoc) {
            this.value = value;
            this.leftAssoc = leftAssoc;
        }
    }

    /**
     * 标志符
     */
    public static class Operators extends HashMap<String, Precedence> {
        public static boolean LEFT = true;

        public static boolean RIGHT = false;

        public void add(String name, int pres, boolean leftAssoc) {
            put(name, new Precedence(pres, leftAssoc));
        }
    }

    protected static class Expr extends Element {
        protected Factory factory;

        protected Operators ops;

        protected BnfParser factor;

        public Expr(Class<? extends AstNode> clazz, BnfParser factor, Operators ops) {
            this.factory = Factory.getForAstList(clazz);
            this.factor = factor;
            this.ops = ops;
        }

        @Override
        protected void parse(HobbyLexer lexer, List<AstNode> nodes) throws ParseException {
            AstNode right = factor.parse(lexer);

            Precedence prec;

            while ((prec = nextOperator(lexer)) != null) {
                right = doShift(lexer, right, prec.value);
            }

            nodes.add(right);
        }

        private AstNode doShift(HobbyLexer lexer, AstNode left, int prec) throws ParseException {
            ArrayList<AstNode> list = new ArrayList<>();
            list.add(left);

            list.add(new AstLeaf(lexer.read()));

            AstNode right = factor.parse(lexer);

            Precedence next;

            while ((next = nextOperator(lexer)) != null && rightIsExpr(prec, next)) {
                right = doShift(lexer, right, next.value);
            }

            list.add(right);

            return factory.make(list);
        }

        private Precedence nextOperator(HobbyLexer lexer) throws ParseException {
            HobbyToken token = lexer.peek(0);

            if (token.isIdentifier()) {
                return ops.get(token.getText());
            } else {
                return null;
            }
        }

        private static boolean rightIsExpr(int prec, Precedence nextPrec) {
            if (nextPrec.leftAssoc) {
                return prec < nextPrec.value;
            } else {
                return prec <= nextPrec.value;
            }
        }

        @Override
        protected boolean match(HobbyLexer lexer) throws ParseException {
            return factor.match(lexer);
        }
    }

    public static final String factoryName = "create";

    protected abstract static class Factory {

        protected abstract AstNode make0(Object arg) throws Exception;

        protected AstNode make(Object arg) {
            try {
                return make0(arg);
            } catch (IllegalArgumentException e1) {
                throw e1;
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        }

        protected static Factory getForAstList(Class<? extends AstNode> clazz) {
            Factory f = get(clazz, List.class);

            if (f == null) {
                f = new Factory() {
                    @Override
                    protected AstNode make0(Object arg) throws Exception {
                        List<AstNode> results = (List<AstNode>) arg;

                        if (results.size() == 1) {
                            return results.get(0);
                        } else {
                            return new AstList(results);
                        }
                    }
                };
            }

            return f;
        }

        protected static Factory get(Class<? extends AstNode> clazz,
                                     Class<?> argType) {
            if (clazz == null) {
                return null;
            }

            try {
                final Method m = clazz.getMethod(factoryName, new Class<?>[]{argType});

                return new Factory() {
                    @Override
                    protected AstNode make0(Object arg) throws Exception {
                        return (AstNode) m.invoke(null, arg);
                    }
                };

            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            }

            try {
                final Constructor<? extends AstNode> c = clazz.getConstructor(argType);

                return new Factory() {
                    @Override
                    protected AstNode make0(Object arg) throws Exception {
                        return c.newInstance(arg);
                    }
                };

            } catch (NoSuchMethodException e) {
                throw new RuntimeException(e);
            }
        }
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
        elements.add(new Tree(parser));
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
