import java.io.IOException;

/**
 * Created by liufengkai on 16/3/14.
 */
public class Parse {
    static int lookahead;

    public Parse() {
        try {
            lookahead = System.in.read();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * 解析表达式
     */
    void expr() {
        term();
        while (true) {
            if (lookahead == '+') {
                match('+');
                term();
                System.out.print('+');
            } else if (lookahead == '-') {
                match('-');
                term();
                System.out.print('-');
            } else
                return;
        }
    }

    /**
     * 判断是不是数位
     */
    void term() {
        if (Character.isDigit((char) lookahead)) {
            System.out.write((char) lookahead);
            match(lookahead);
        } else
            throw new Error("syntax error");
    }

    void match(int t) {
        if (lookahead == t) {
            try {
                lookahead = System.in.read();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else
            throw new Error("syntax error");
    }

    public static class PostFix{
        public static void main(String[] args){
            Parse parse = new Parse();
            parse.expr();
            System.out.print('\n');
        }
    }
}
