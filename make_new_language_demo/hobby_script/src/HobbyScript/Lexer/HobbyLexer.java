package HobbyScript.Lexer;

import HobbyScript.Exception.ParseException;
import HobbyScript.Token.HobbyToken;
import HobbyScript.Token.IdToken;
import HobbyScript.Token.NumberToken;
import HobbyScript.Token.StringToken;

import java.io.IOException;
import java.io.LineNumberReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Lexer Hobby的词法分析器
 * Created by liufengkai on 16/7/11.
 */
public class HobbyLexer {
    private Pattern regPattern = Pattern.compile(HobbyRegex.hobbyReg);

    private ArrayList<HobbyToken> queue = new ArrayList<>();

    private boolean hasMore;

    private LineNumberReader reader;

    /**
     * 构造
     *
     * @param reader 传入的Reader加载字符流
     */
    public HobbyLexer(Reader reader) {
        this.hasMore = true;
        this.reader = new LineNumberReader(reader);
    }

    /**
     * 读取Token队列的下一个Token
     *
     * @return 返回第一个Token
     * @throws ParseException
     */
    public HobbyToken read() throws ParseException {
        if (fillQueue(0)) {
            return queue.remove(0);
        } else {
            return HobbyToken.EOF;
        }
    }


    public HobbyToken peek(int index) throws ParseException {
        if (fillQueue(index)) {
            return queue.get(index);
        } else {
            return HobbyToken.EOF;
        }
    }

    private boolean fillQueue(int index) throws ParseException {
        while (index >= queue.size()) {
            if (hasMore) {
                readLine();
            } else {
                return false;
            }
        }
        return true;
    }

    private void readLine() throws ParseException {
        String line;

        try {
            line = reader.readLine();
        } catch (IOException e) {
            throw new ParseException(e);
        }

        if (line == null) {
            hasMore = false;
            return;
        }

        int lineNum = reader.getLineNumber();

        Matcher matcher = regPattern.matcher(line);
        matcher.useTransparentBounds(true)
                .useAnchoringBounds(true);

        int start = 0, end = line.length();

        while (start < end) {
            matcher.region(start, end);
            if (matcher.lookingAt()) {
                addToken(lineNum, matcher);
                start = matcher.end();
            } else {
                throw new ParseException("bad token at line " + lineNum);
            }

            queue.add(new IdToken(lineNum, HobbyToken.EOL));
        }
    }

    private void addToken(int lineNum, Matcher matcher) {
        String match = matcher.group(1);

        if (match != null) {
            if (matcher.group(2) == null) {
                HobbyToken token;

                if (matcher.group(3) != null) {
                    token = new NumberToken(lineNum, Integer.parseInt(match));
                } else if (matcher.group(4) != null) {
                    token = new StringToken(lineNum, toStringLiteral(match));
                } else {
                    token = new IdToken(lineNum, match);
                }

                queue.add(token);
            }
        }
    }

    private String toStringLiteral(String str) {
        StringBuilder builder = new StringBuilder();

        int length = str.length() - 1;

        for (int i = 1; i < length; i++) {
            char ch = str.charAt(i);

            if (ch == '\\' && i + 1 < length) {
                int ch2 = str.charAt(i + 1);

                if (ch2 == '"' || ch2 == '\\') {
                    ch = str.charAt(++i);
                } else {
                    ++i;
                    ch = '\n';
                }
            }
            builder.append(ch);
        }
        return builder.toString();
    }
}
