package HobbyScript.Lexer;

/**
 * 匹配词素的默认正则表达式
 * <p>
 * 脚本语言么,就是要怎么简单怎么来么,
 * 谁他么在乎什么效率,所以我使用正则来匹配Token
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/11.
 */
public class HobbyRegex {
    /**
     * 匹配注释
     * 即注释线之后+任意数量的字符
     */
    public static final String annotationReg = "//.*";
    public static final String hobbyReg = "\s*()?";
}
