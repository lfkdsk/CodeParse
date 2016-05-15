import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

/**
 * Created by liufengkai on 16/5/14.
 */
public class ThirdTimeFirstFollow {

    public static String testConfigFile = "/Users/liufengkai/" +
            "Documents/JavaProject/FrontHomeWork/src/ConfigFile";


    public static void main(String[] args) throws IOException {
        BufferedReader builder = new BufferedReader(new FileReader(testConfigFile));

        String temp;

        while ((temp = builder.readLine()) != null) {
            // 删除空格 录入分析
            FrontDefault.words.add(new Word(temp.replace(" ", "")));
        }
        // 添加字母表
        FirstSet.addFixedWord();

        System.out.println("读取完毕");

        // 打印字母表
        FrontDefault.words.forEach(Word::printWord);

        FirstSet.First("A").printWordSet();

        FollowSet.Follow("B").printWordSet();
    }
}
