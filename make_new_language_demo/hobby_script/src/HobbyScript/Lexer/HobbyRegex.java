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
    public static final String annotationReg = "(//.*)";

    /**
     * 数字类型的正则
     * 目前居然只能支持int类型真垃圾
     */
    public static final String numberReg = "([0-9]+)";

    /**
     * 变量名的正则
     */
    public static final String variableReg = "[A-Z_a-z][A-Z_a-z0-9]*";

    /**
     * 布尔表达式的正则
     */
    public static final String booleanReg = "==|<=|>=|&&|\\|\\|";

    /**
     * 任意符号的正则匹配
     */
    public static final String symbolReg = "\\p{Punct}";

    /**
     * string 类型的正则
     * string 中是在两个双引号中的多个匹配模式
     * 支持 \" \n \\ 和任意一种不是"的符号匹配
     * 遇到\\"这种试图二度转译的应当会先与\\匹配
     * 使转译停止
     */
    public static final String stringReg = "(\"(\\\\\"|\\\\\\\\|\\\\n|[^\"])*\")";

    /**
     * 定义了所有Token识别的基础类型
     */
    public static final String tokenReg = variableReg + "|" + booleanReg + "|" + symbolReg;

    public static final String hobbyUnFormat = "\\s*(%1$s|%2$s|%3$s|%4$s)?";

    public static final String hobbyReg = String.format(hobbyUnFormat,
            annotationReg, numberReg, stringReg, tokenReg);

    /**
     * 控制枚举自增
     */
    private static int INDEX_COUNT = 0;

    /**
     * 各部分Token在正则中的位置
     * matcher.group的选取是看括号的
     * 前套来的首先匹配的是全局,
     * 然后缩紧一次括号位置
     */
    public enum RegType {
        ALL_INDEX,
        NOT_EMPTY_INDEX,
        ANNOTATION_INDEX,
        NUMBER_INDEX,
        STRING_INDEX,
        SYMBOL_INDEX,
        VARIABLE_INDEX,
        BOOLEAN_INDEX,
        OTHER_SYMBOL_INDEX;

        public int indexNum;

        RegType() {
            this.indexNum = INDEX_COUNT++;
        }
    }

    public static void main(String[] args) {
        // 全部的正则表达式
        System.out.println(hobbyReg);
    }
}
