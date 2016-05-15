import java.util.ArrayList;

/**
 * Created by liufengkai on 16/5/14.
 */
public class Word {
    public String left;

    public String[] wordTypes;

    public Word(String word) {
        String[] exp = word.split("->");
        left = exp[0];
        wordTypes = exp[1].split("\\|");
    }

    /**
     * 产生式右部包含非终结符
     *
     * @param X 非终结
     * @return
     */
    public boolean contains(String X) {
        for (int i = 0; i < wordTypes.length; i++) {
            if (wordTypes[i].contains(X)) {
                return true;
            }
        }
        return false;
    }

    public ArrayList<String> getWordTypeContain(String X) {
        ArrayList<String> temp = new ArrayList<>();
        for (int i = 0; i < wordTypes.length; i++) {
            if (wordTypes[i].contains(X)) {
                temp.add(wordTypes[i]);
            }
        }
        return temp;
    }

    public void printWord() {
        System.out.println(left + " :->");
        for (String t : wordTypes) {
            System.out.print(" | " + t);
        }
        System.out.println();
        System.out.println("对应产生式的数目 :" + wordTypes.length);
    }
}
