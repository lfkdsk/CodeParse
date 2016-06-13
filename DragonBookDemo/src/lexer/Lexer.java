package lexer;

import symbols.Type;

import java.io.IOException;
import java.util.Hashtable;

/**
 * 词法分析器
 * String --> Token
 * Created by liufengkai on 16/3/16.
 */
public class Lexer {
    // 行号
    public static int line = 1;
    // 预读一个字符
    char peek = ' ';

    Hashtable<String, Word> words = new Hashtable<>();

    void reserve(Word word) {
        words.put(word.lexeme, word);
    }

    // 填充一下=_=
    public Lexer() {
        reserve(new Word("if", Tag.IF));
        reserve(new Word("else", Tag.ELSE));
        reserve(new Word("while", Tag.WHILE));
        reserve(new Word("do", Tag.DO));
        reserve(new Word("break", Tag.BREAK));
        reserve(Word.True);
        reserve(Word.False);
        reserve(Type.Int);
        reserve(Type.Float);
        reserve(Type.Char);
        reserve(Type.Bool);
    }

    // 预读下一个字符到peek中
    void readch() throws IOException {
//        try {
            peek = (char) System.in.read();
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
    }

    // 识别复合单元,预读取下一个内容
    boolean readch(char c) throws IOException{
        readch();
        if (peek != c)
            return false;

        peek = ' ';
        return true;
    }

    /**
     * 词素的扫描
     *
     * @return
     */
    public Token scan() throws IOException {
        // 跳过空白行/字符/制表符
        for (; ; readch()) {
            if (peek == ' ' || peek == '\t')
                continue;
            else if (peek == '\n')
                line = line + 1;
            else break;
        }

        // 在这里处理了预读的下一个字符,判断而不能组合出二元符
        // 这里在试图识别双目运算符
        switch (peek) {
            case '&':
                if (readch('&')) return Word.and;
                else return new Token('&');
            case '|':
                if (readch('|')) return Word.or;
                else return new Token('|');
            case '=':
                if (readch('=')) return Word.eq;
                else return new Token('=');
            case '!':
                if (readch('=')) return Word.ne;
                else return new Token('!');
            case '<':
                if (readch('=')) return Word.le;
                else return new Token('<');
            case '>':
                if (readch('=')) return Word.ge;
                else return new Token('>');
        }

        // 处理数字
        if (Character.isDigit(peek)) {
            int v = 0;
            do {
                v = 10 * v + Character.digit(peek, 10);
                readch();
            } while (Character.isDigit(peek));

            // 小数
            if (peek != '.')
                return new Num(v);

            float x = v;
            float d = 10;

            for (; ; ) {
                readch();
                if (!Character.isDigit(peek))
                    break;
                x = x + Character.digit(peek, 10) / d;
                d = d * 10;
            }
            return new Real(x);
        }

        // 读入字符串
        if (Character.isLetter(peek)) {
            StringBuilder buffer = new StringBuilder();
            do {
                buffer.append(peek);
                readch();
            } while (Character.isLetterOrDigit(peek));

            String s = buffer.toString();

            Word w = words.get(s);

            if (w != null) return w;

            w = new Word(s, Tag.ID);
            words.put(s, w);
            return w;
        }

        Token token = new Token(peek);
        peek = ' ';
        return token;
    }

}
