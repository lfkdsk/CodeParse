package HobbyScript.Lexer;

import HobbyScript.Token.HobbyToken;

import java.io.LineNumberReader;
import java.io.Reader;
import java.util.ArrayList;
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


}
