package HobbyScript.Compile;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;

/**
 * 代码段的存储
 * <p>
 * 语法声明:
 * <p>
 * <p>
 * <p>
 * Created by liufengkai on 16/8/7.
 */
public class CodeLine {
    private int codeLine = 0;

    private Code prevCode = null;

    private class Code implements Comparable<Code> {
        public String line;
        public int lineNum;

        public Code(String line, int lineNum) {
            this.line = line;
            this.lineNum = lineNum;
        }

        @Override
        public int compareTo(Code o) {
            if (this.lineNum < o.lineNum) return -1;
            else if (this.lineNum == o.lineNum) return 0;
            else return 1;
        }

        @Override
        public String toString() {
            if (lineNum != -1)
                return "L" + lineNum + " :" + line;
            else
                return "\t" + line;
        }
    }

    /**
     * 脚本的转换代码
     */
    private ArrayList<Code> codes = new ArrayList<>();

    public CodeLine() {

    }

    public void addCode(String line, int codeLine) {
        if (prevCode != null) {
            prevCode.line = line;
            codes.add(prevCode);
            prevCode = null;
            return;
        }
        codes.add(new Code(line, codeLine));
    }

    public void addSpecCode(String line, int codeLine) {
        codes.add(new Code(line, codeLine));
    }

    public void addCode(String line) {
        addCode(line, -1);
    }

    public void deleteCode(Code line) {
        codes.remove(line);
    }

    public Code getCode(int index) {
        if (index < 0) {
            return codes.get(codes.size() + index);
        } else {
            return codes.get(index);
        }
    }

    public int newLine() {
        return ++codeLine;
    }

    public void printList() {
        for (int i = 0; i < codes.size(); i++) {
            System.out.println(codes.get(i).toString());
        }
    }

    public void addPrevCode(int codeLine) {
        prevCode = new Code("", codeLine);
    }

    public void clearPrevCode() {
        prevCode = null;
    }

    public void writeToFile() throws IOException {
        File file = new File("../hobby_script/MiddleCode/" + System.currentTimeMillis());
        if (file.createNewFile()){
            BufferedWriter writer = new BufferedWriter(new FileWriter(file));

            for (int i = 0; i < codes.size(); i++) {
                writer.write(codes.get(i).toString());
                writer.newLine();
            }

            writer.close();
        }
    }
}
