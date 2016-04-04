package parser;

import inter.*;
import lexer.*;
import symbols.Array;
import symbols.Env;
import symbols.Type;

import java.io.IOException;

/**
 * 语法分析器
 * Created by liufengkai on 16/3/17.
 */
public class Parser {
    // 词法分析器
    private Lexer lexer;
    // 词法分析起阅读
    private Token look;
    // 顶层符号表
    Env top = null;
    // 变量声明的存储位置
    int used = 0;

    public Parser(Lexer lexer) throws IOException {
        this.lexer = lexer;
        move();
    }

    void move() throws IOException {
        look = lexer.scan();
    }

    void error(String s) {
        throw new Error("near line " + Lexer.line + ": " + s);
    }

    void match(int t) throws IOException {
        if (look.tag == t) move();
        else error("syntax error");
    }

    /**
     * run
     * @throws IOException
     */
    public void program() throws IOException {
        // 读取块
        Stmt s = block();
        int begin = s.newLabel();
        int after = s.newLabel();
        s.emitLabel(begin);
        s.gen(begin, after);
        s.emitLabel(after);
    }

    // block -> {decls stmt}
    Stmt block() throws IOException {
        match('{');
        // 字符表的树
        Env saveEnv = top;
        top = new Env(saveEnv);

        decls();

        // 生成语句序列
        Stmt s = stmts();
        // 匹配结尾再见
        match('}');
        // 恢复外层符号表
        top = saveEnv;
        return s;
    }

    // D -> type ID

    /**
     * 匹配初始化 ID
     * @throws IOException
     */
    void decls() throws IOException {
        while (look.tag == Tag.BASIC) {
            Type p = type();
            Token token = look;
            match(Tag.ID);
            match(';');
            ID id = new ID(token, p, used);
            top.put(token, id);
            used = used + p.width;
        }
    }

    /**
     * 实例化一个BASIC类型/数组类型
     * @return
     * @throws IOException
     */
    Type type() throws IOException {
        Type p = (Type) look;
        match(Tag.BASIC);
        if (look.tag != '[') return p;
        else return dims(p);
    }

    /**
     * 实例化数组对象
     * @param p
     * @return
     * @throws IOException
     */
    Type dims(Type p) throws IOException {
        match('[');
        Token token = look;
        match(Tag.NUM);
        match(']');
        if (look.tag == '[')
            p = dims(p);
        return new Array(((Num) token).value, p);
    }

    Stmt stmts() throws IOException {
        if (look.tag == '}')
            return Stmt.Null;
        // 进行不断的递归语句序列
        else return new Seq(stmt(), stmts());
    }

    /**
     * 匹配非终结表达式
     * @return
     * @throws IOException
     */
    Stmt stmt() throws IOException {
        Expr x;
        Stmt s1, s2;
        Stmt savedStmt; // break 保存外层语句
        switch (look.tag) {
            case ';':
                move();
                return Stmt.Null;
            case Tag.IF:
                match(Tag.IF);
                match('(');
                x = bool();
                match(')');
                s1 = stmt();
                if (look.tag != Tag.ELSE) return new If(x, s1);
                match(Tag.ELSE);
                s2 = stmt();
                return new Else(x, s1, s2);

            case Tag.WHILE:
                While whilenode = new While();
                savedStmt = Stmt.Enclosing;
                Stmt.Enclosing = whilenode;
                match(Tag.WHILE);
                match('(');
                x = bool();
                match(')');
                s1 = stmt();
                whilenode.init(x, s1);
                Stmt.Enclosing = savedStmt; // reset
                return whilenode;

            case Tag.DO:
                Do donode = new Do();
                savedStmt = Stmt.Enclosing;
                Stmt.Enclosing = donode;
                match(Tag.DO);
                s1 = stmt();
                match(Tag.WHILE);
                match('(');
                x = bool();
                match(')');
                match(';');
                donode.init(s1, x);
                Stmt.Enclosing = savedStmt;
                return donode;
            case Tag.BREAK:
                match(Tag.BREAK);
                match(';');
                return new Break();
            case '{':
                return block();
            default:
                return assign();
        }
    }

    /**
     * 赋值语句
     *
     * @return
     * @throws IOException
     */
    Stmt assign() throws IOException {
        Stmt stmt;
        Token token = look;
        match(Tag.ID);
        ID id = top.get(token);
        if (id == null) error(token.toString() + " undeclared");
        if (look.tag == '=') {// S -> id = E
            move();
            stmt = new Set(id, bool());
        } else {// S -> L = E
            Access x = offset(id);
            match('=');
            stmt = new SetElem(x, bool());
        }
        match(';');
        return stmt;
    }

    /**
     * or
     * @return
     * @throws IOException
     */
    Expr bool() throws IOException {
        Expr x = join();
        while (look.tag == Tag.OR) {
            Token tok = look;
            move();
            x = new Or(tok, x, join());
        }
        return x;
    }

    /**
     * and
     * @return
     * @throws IOException
     */
    Expr join() throws IOException {
        Expr x = equality();
        while (look.tag == Tag.AND) {
            Token tok = look;
            move();
            x = new And(tok, x, equality());
        }
        return x;
    }

    /**
     * 双目 条件
     * @return
     * @throws IOException
     */
    Expr equality() throws IOException {
        Expr x = rel();
        while (look.tag == Tag.EQ || look.tag == Tag.NE) {
            Token tok = look;
            move();
            x = new Rel(tok, x, rel());
        }
        return x;
    }

    /**
     * 条件
     * @return
     * @throws IOException
     */
    Expr rel() throws IOException {
        Expr x = expr();
        switch (look.tag) {
            case '<':
            case Tag.LE:
            case Tag.GE:
            case '>':
                Token tok = look;
                move();
                return new Rel(tok, x, expr());
            default:
                return x;
        }
    }

    /**
     * + / -
     * @return
     * @throws IOException
     */
    Expr expr() throws IOException {
        Expr x = term();
        while (look.tag == '+' || look.tag == '-') {
            Token tok = look;
            move();
            x = new Arith(tok, x, unary());
        }
        return x;
    }

    /**
     * 单目 * / /
     * @return
     * @throws IOException
     */
    Expr term() throws IOException {
        Expr x = unary();
        while (look.tag == '*' || look.tag == '/') {
            Token tok = look;
            move();
            x = new Arith(tok, x, unary());
        }
        return x;
    }

    /**
     * 单目 - / !
     * @return
     * @throws IOException
     */
    Expr unary() throws IOException {
        if (look.tag == '-') {
            move();
            return new Unary(Word.minus, unary());
        } else if (look.tag == '!') {
            Token tok = look;
            move();
            return new Not(tok, unary());
        } else return factor();
    }

    /**
     * 类型匹配
     * @return
     * @throws IOException
     */
    Expr factor() throws IOException {
        Expr x = null;
        switch (look.tag) {
            case '(':
                match('(');
                x = bool();
                match(')');
                return x;
            case Tag.NUM:
                x = new Constant(look, Type.Int);
                move();
                return x;
            case Tag.REAL:
                x = new Constant(look, Type.Float);
                move();
                return x;
            case Tag.TRUE:
                x = Constant.True;
                move();
                return x;
            case Tag.FALSE:
                x = Constant.False;
                move();
                return x;
            default:
                error("syntax error");
                return x;
            case Tag.ID:
                ID id = top.get(look);
                if (id == null) error(look.toString() + " undeclared");
                move();
                if (look.tag != '[') return id;
                else return offset(id);
        }
    }

    /**
     * 匹配数组对象
     * @param id
     * @return
     * @throws IOException
     */
    // I --> [E] | [E] I
    Access offset(ID id) throws IOException {
        Expr i, w, t1, t2, loc;
        Type type = id.type;
        match('[');
        i = bool();         // I -> [E]
        match(']');
        type = ((Array) type).of;
        w = new Constant(type.width);
        t1 = new Arith(new Token('*'), i, w);
        loc = t1;
        // 生成多维下标
        while (look.tag == '[') {
            match('[');
            i = bool();
            match(']');
            type = ((Array) type).of;
            w = new Constant(type.width);
            t1 = new Arith(new Token('*'), i, w);
            t2 = new Arith(new Token('+'), loc, t1);
            loc = t2;
        }
        return new Access(id, loc, type);
    }
}
