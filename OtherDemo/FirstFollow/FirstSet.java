import java.util.Collections;

/**
 * Created by liufengkai on 16/5/15.
 */
public class FirstSet {

    public static void addFixedWord() {
        String[] temp = {"a", "b", "c", "d", "e", "$"};
        Collections.addAll(FrontDefault.fixedWords, temp);
    }

    public static WordSet First(String X) {
        WordSet set = new WordSet(X);
        // 判断终结符号
        if (FrontDefault.fixedWords.contains(X)) {
            set.add(X);
            return set;
        }

        for (Word word : FrontDefault.words) {
            if (word.left.equals(X)) {
                for (int i = 0; i < word.wordTypes.length; i++) {
                    // 终结符号
                    String firstChar = String.valueOf(word.wordTypes[i].charAt(0));
                    if (FrontDefault.fixedWords.
                            contains(firstChar)) {
                        set.add(firstChar);
                        // 判断 $
                    } else if (firstChar.equals(FrontDefault.endWord) &&
                            word.wordTypes[i].length() == 1) {
                        set.add(word.wordTypes[i]);
                    } else {
                        // 产生式中没法写分词 所以直接默认单个字符
                        // 对非终结符号求First集
                        set.add(getFirstFromNonEnd(word.wordTypes[i]));
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
                // First子集没有 $
                if (!subSet.contains(FrontDefault.endWord)) {
                    isEndWord[k] = false;
                    set.add(subSet);
                    return set;
                } else {
                    // 自己还有 $
                    isEndWord[k] = true;
                    // 删掉 $
                    subSet.delete(FrontDefault.endWord);
                    set.add(subSet);
                }
            }
        }


        // 检查一下 是不是所有的都有 $
        for (int i = 0; i < isEndWord.length; i++) {
            if (isEndWord[i]) {
                set.add(FrontDefault.endWord);
            }
        }
        return set;
    }
}
