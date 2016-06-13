import java.util.ArrayList;

/**
 *
 * Created by liufengkai on 16/5/14.
 */
public class WordSet {
    public String left = "";

    private ArrayList<String> words;

    // temp 用的
    public WordSet() {
        words = new ArrayList<>();
    }

    public WordSet(String X) {
        left = X;
        words = new ArrayList<>();
    }

    public void add(String word) {
        if (!words.contains(word)) {
            words.add(word);
        }
    }

    public ArrayList<String> getWords() {
        return words;
    }

    public void add(WordSet set) {
        for (int i = 0; i < set.getWords().size(); i++) {
            add(set.getWords().get(i));
        }
    }

    public boolean contains(String word) {
        return words.contains(word);
    }

    public void delete(String word) {
        if (words.contains(word)) {
            words.remove(word);
        }
    }

    public void printWordSet() {
        System.out.println();
        System.out.print("{");
        for (int i = 0;i < words.size() ;i++) {
            System.out.print(words.get(i));
            if (i != words.size() - 1)
                System.out.print(" , ");
        }
        System.out.print("}");
    }
}
