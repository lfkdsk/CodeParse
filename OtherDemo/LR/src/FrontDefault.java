import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

/**
 * Created by liufengkai on 16/5/15.
 */
public class FrontDefault {

    public static ArrayList<CorExpression> expressions = new ArrayList<>();
    // 终结符字母表
    public static ArrayList<String> fixedWords = new ArrayList<>();
    // 非终结字母表
    public static ArrayList<String> unfixedWords = new ArrayList<>();

    public static HashMap<String, WordSet> firstSets = new HashMap<>();

    public static final String emptyWord = "æ";

    public static final String endWord = "$";

    public static final String startPoint = "S";

    public static void addWord() {
        String[] fixed = {"$", "(", ")", "*", "+", "i"};
        String[] unfixed = {"E", "T", "F"};
        Collections.addAll(fixedWords, fixed);
        Collections.addAll(unfixedWords, unfixed);
    }

    public static int getSymPostion(String type) {
        int index = 0;
        for (String ini : fixedWords) {

            if (ini.equals(type)) {
                return index;
            }
            index++;
        }

        for (String ini : unfixedWords) {
            if (ini.equals(type)) {
                return index;
            }
            index++;
        }

        return index;
    }

    public static void getFirstSet() {
        for (String fixedWord : fixedWords) {
            firstSets.put(fixedWord, FirstSet.getFirstSet(fixedWord));
        }

        for (String unfixedWord : unfixedWords) {
            firstSets.put(unfixedWord, FirstSet.getFirstSet(unfixedWord));
        }
    }

    public static void printFirstTable() {
        for (String word : firstSets.keySet()) {
            System.out.println(word + "的first集:");
            firstSets.get(word).printWordSet();
            System.out.println();
        }
    }

    public static Expression getPointExpression(int number) {
        int allNum = 0;
        Expression expression = null;

        if (number == 0)
            return expressions.get(0);

        for (int i = 0; i < expressions.size(); i++) {
            allNum += expressions.get(i).wordTypes.length;
            if (number == allNum) {
                expression = expressions.get(i + 1);
                break;
            }

            if (number < allNum) {
                expression = expressions.get(i);
                break;
            }
        }

        if (expression != null) {
            expression.backNumber = allNum == number ? 0 : allNum - number;
        }

        return expression;
    }

    public static String getPointString(int number) {
        Expression expression = getPointExpression(number);

        return (expression != null ?
                expression.wordTypes[expression.backNumber] : "fuck error");
    }

    public static String printExpression(int number) {
        Expression expression = getPointExpression(number);
        return (expression != null ?
                expression.left + "->" + expression.wordTypes[expression.backNumber]
                : "fuck error");
    }

    public static ArrayList<Expression> getExpressionStartWith(String x) {
        ArrayList<Expression> exs = new ArrayList<>();
        for (Expression expression : expressions) {
            if (expression.left.equals(x)) {
                exs.add(expression);
            }
        }
        return exs;
    }

    public static int getExpressionLength(ArrayList<Expression> exs) {
        int all = 0;
        for (Expression expression : exs) {
            all += expression.wordTypes.length;
        }
        return all;
    }
}

