import java.util.ArrayList;

/**
 * Created by liufengkai on 16/5/23.
 */
public class CorExpression extends Expression implements Cloneable {
    // 点指针
    public int[] pointer;
    // 搜索符序列
    public ArrayList<ArrayList<String>> pointerSet;

    public CorExpression() {

    }

    public CorExpression(String word) {
        super(word);

        pointer = new int[wordTypes.length];
        // initial
        for (int i = 0; i < pointer.length; i++) {
            pointer[i] = 0;
        }

        pointerSet = new ArrayList<>();
    }

    @Override
    public CorExpression clone() {
        CorExpression ex = new CorExpression();
        ex.pointerSet = new ArrayList<>(pointerSet);
        ex.pointer = new int[pointer.length];
        ex.wordTypes = new String[wordTypes.length];

        for (int i = 0; i < pointer.length; i++) {
            ex.pointer[i] = pointer[i];
            ex.wordTypes[i] = wordTypes[i];
        }

        ex.left = left;
        return ex;
    }
}
