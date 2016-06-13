import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 * Created by liufengkai on 16/6/11.
 */
public class HomeWorkFive {
    public static String testConfigFile = "/Users/liufengkai/Documents/JavaProject/LR/src/Config";

    public static void main(String[] args) throws IOException {
        BufferedReader builder = new BufferedReader(new FileReader(testConfigFile));

        String temp;

        // 扩展文法
        CorExpression s = new CorExpression("S->E");
        s.number = 0;
        FrontDefault.expressions.add(s);

        int index = 1;
        while ((temp = builder.readLine()) != null) {
            CorExpression exp = new CorExpression(temp.replace(" ", ""));
            // 删除空格 录入分析
            exp.number = index;
            FrontDefault.expressions.add(exp);
            index += exp.wordTypes.length;
        }
        // 添加词素
        FrontDefault.addWord();
//        FrontDefault.expressions.forEach(Expression::printWord);
        FrontDefault.getFirstSet();
//        FrontDefault.printFirstTable();
        ProjectItem.getProjectItems();
        ProjectItem.printLRList();

        ProjectItem.getTable();
        ProjectItem.printTable();
        ProjectItem.analyze("(i)");
    }
}
