import java.util.ArrayList;

/**
 * Created by liufengkai on 16/5/15.
 */
public class FirstSet {
    public static ArrayList<String> usedWord = new ArrayList<>();

    public static WordSet getFirstSet(String x) {
        WordSet set = First(x);
        usedWord.clear();
        return set;
    }

    private static WordSet First(String X) {
        WordSet set = new WordSet(X);

        // 判断终结符号
        if (FrontDefault.fixedWords.contains(X)) {
            set.add(X);
            return set;
        }

        // 非终结符号
        if (usedWord.contains(X)) {
            return FrontDefault.firstSets.get(X) == null ? set
                    : FrontDefault.firstSets.get(X);
        } else {
            usedWord.add(X);
        }

        for (Expression expression : FrontDefault.expressions) {
            if (expression.left.equals(X)) {
                for (int i = 0; i < expression.wordTypes.length; i++) {
                    String firstChar = String.valueOf(expression.wordTypes[i].charAt(0));
                    if (FrontDefault.fixedWords.
                            contains(firstChar)) {
                        set.add(firstChar);
                        // 判断
                    } else if (firstChar.equals(FrontDefault.emptyWord) &&
                            expression.wordTypes[i].length() == 1) {
                        set.add(expression.wordTypes[i]);
                    } else {
                        // 产生式中没法写分词 所以直接默认单个字符
                        // 对非终结符号求First集
                        set.add(getFirstFromNonEnd(expression.wordTypes[i]));
                    }
                }
            }
        }
        return set;
    }

    /**
     * 从剩余的产生右部继续求First集
     *
     * @param wordTypes 分割出的下一部分
     * @return
     */
    public static WordSet getFirstFromNonEnd(String wordTypes) {
        // 申请判断数组
        boolean[] isEndWord = new boolean[wordTypes.length()];
        WordSet set = new WordSet();
        // 循环判断
        for (int k = 0; k < wordTypes.length(); k++) {
            // 拿到每个首字母
            String subFirst = String.valueOf(wordTypes.charAt(k));
            // 终结符
            if (FrontDefault.fixedWords.contains(subFirst)) {
                set.add(subFirst);
                return set;
            }
            // 求他的First集
            WordSet subSet = First(subFirst);
            if (subSet != null) {
                // First子集没有
                if (!subSet.contains(FrontDefault.emptyWord)) {
                    isEndWord[k] = false;
                    set.add(subSet);
                    return set;
                } else {
                    // 自己还有
                    isEndWord[k] = true;
                    // 删掉
                    subSet.delete(FrontDefault.emptyWord);
                    set.add(subSet);
                }
            }
        }


        // 检查一下 是不是所有的都有 $
        for (int i = 0; i < isEndWord.length; i++) {
            if (isEndWord[i]) {
                set.add(FrontDefault.emptyWord);
            }
        }
        return set;
    }
}
