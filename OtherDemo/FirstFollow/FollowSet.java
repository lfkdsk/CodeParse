import java.util.ArrayList;

/**
 * Created by liufengkai on 16/5/15.
 */
public class FollowSet {

    public static ArrayList<String> firstFollow = new ArrayList<>();

    public static WordSet Follow(String X) {
        return Follows(X);
    }

    public static WordSet Follows(String X) {
        WordSet set = new WordSet(X);

        if (FrontDefault.fixedWords.contains(X))
            return set;
        else
            firstFollow.add(X);
        // 判断是不是文法开始符号
        if (FrontDefault.words.get(0).left.equals(X)) {
            set.add(FrontDefault.endExpression);
        }

        for (Word word : FrontDefault.words) {
            // 找到有产生式的右部
            if (word.contains(X)) {
                ArrayList<String> containList = word.getWordTypeContain(X);
                // 判断状态
                for (int i = 0; i < containList.size(); i++) {
                    String contain = containList.get(i);

                    int index = contain.indexOf(X);
                    // 本行最后一个符号 左Follow加入
                    if (index == contain.length() - 1) {
                        if (!firstFollow.contains(word.left))
                            set.add(Follows(word.left));
                    } else {
                        String nextChar = String.valueOf(contain.charAt(index + 1));
                        // 终结
                        if (FrontDefault.fixedWords.contains(nextChar)) {
                            set.add(nextChar);
                        } else {
                            WordSet subFirst = FirstSet.First(nextChar);
                            if (subFirst != null) {
                                if (subFirst.contains(FrontDefault.endWord)
                                        && !firstFollow.contains(subFirst.left)) {
                                    set.add(Follows(subFirst.left));
                                }
                                // 删掉空
                                subFirst.delete(FrontDefault.endWord);
                                set.add(subFirst);
                            }
                        }
                    }
                }
            }
        }

        return set;
    }

}
