import java.util.*;

/**
 * Created by liufengkai on 16/6/11.
 */
public class ProjectItem {
    public static List<List<LRItem>> LRList = new ArrayList<>();

    public static List<Link> links = new ArrayList<>();

    private static TableItem[][] table = new TableItem[15][15];

    /**
     * 开始产生式的入口
     */
    public static void getProjectItems() {
        List<LRItem> items;
        // 获得初始状态集
        LRItem item = new LRItem();
        item.id = 0;
        item.num = 0;
        item.fst.add(FrontDefault.endWord);
        items = getClosure(item);

        LinkedList<List<LRItem>> listQueue = new LinkedList<>();
        listQueue.push(items);
        LRList.add(items);
        while (!listQueue.isEmpty()) {
            // 栈顶的项目
            List<LRItem> curList = listQueue.pop();
            int fixedSize = FrontDefault.fixedWords.size();
            int unfixedSize = FrontDefault.unfixedWords.size();

            for (int i = 0; i < fixedSize + unfixedSize; i++) {
                String point = i < fixedSize ? FrontDefault.fixedWords.get(i)
                        : FrontDefault.unfixedWords.get(i - fixedSize);

                if (point.equals("(")){
                    System.out.println();
                }

                List<LRItem> newList = new ArrayList<>();
                for (int j = 0; j < curList.size(); j++) {
                    // 逐个项目搜索
                    LRItem lrItem = curList.get(j);

                    String pointString = FrontDefault.getPointString(lrItem.num);
                    // 规约
                    if (lrItem.id == pointString.length())
                        continue;
                    // 如果可以移进
                    if (point.equals(String.valueOf(pointString.charAt(lrItem.id == -1 ? 0 : lrItem.id)))) {
                        LRItem newItem = new LRItem();
                        newItem.num = lrItem.num;
                        newItem.id = lrItem.id + 1;
                        newList = mergeList(newList, getClosure(newItem));
                    }
                }

                if (newList.size() == 0) continue;
                int curEdgeNumber = findList(curList, LRList);
                int nextEdgeNumber = findList(newList, LRList);
                // 不存在的节点
                if (nextEdgeNumber == -1) {
                    LRList.add(newList);
                    listQueue.push(newList);
                    links.add(new Link(curEdgeNumber, LRList.size() - 1, point));
                } else {
                    Link link = new Link(curEdgeNumber, nextEdgeNumber, point);
                    if (!isTheLineIn(link)) {
                        links.add(link);
                    }
                }
            }
        }
    }


    /**
     * 求闭包
     * @param item
     * @return
     */
    public static List<LRItem> getClosure(LRItem item) {

        List<LRItem> items = new ArrayList<>();
        items.add(item);

        LinkedList<LRItem> itemsQueue = new LinkedList<>();                         //bfs完成闭包
        itemsQueue.push(item);

        ArrayList<String> usedList = new ArrayList<>();

        while (!itemsQueue.isEmpty()) {
            // 被推倒项
            LRItem front = itemsQueue.pop();
            String pointString = FrontDefault.getPointString(front.num);
            if (pointString.length() == front.id) continue;
            // 拿到下一个加入的项目
            String point = String.valueOf(pointString.charAt(front.id == -1 ? 0 : front.id));
            // 干掉其中的终结符
            if (FrontDefault.fixedWords.contains(point)) {
                continue;
            }

            if (usedList.contains(point)) {
                continue;
            }

            // 拿到被推项目的对应
            ArrayList<Expression> expWithPoint = FrontDefault.getExpressionStartWith(point);
            // 添加非终结项目
            usedList.add(point);
            // 推倒项目
            for (Expression exp : expWithPoint) {
                for (int i = 0; i < exp.wordTypes.length; i++) {
                    LRItem newItem = new LRItem();
                    newItem.id = 0;
                    newItem.num = exp.number + i;
                    // 这是后面啥都没有了
                    if (pointString.length() - front.id == 1) {
                        newItem.fst.addAll(front.fst);
                    } else {
                        newItem.fst.addAll(FrontDefault.firstSets.get(point).getWords());
                    }

                    if (!isInIt(items, newItem)) {
                        itemsQueue.push(newItem);
                        items.add(newItem);
                    }
                }
            }
        }
        return items;
    }

    private static boolean isInIt(List<LRItem> ex, LRItem item) {
        for (LRItem lrItem : ex) {
            if (lrItem.id == item.id
                    && lrItem.num == item.num
                    && lrItem.fst.equals(item.fst)) {
                return true;
            }
        }
        return false;
    }

    /**
     * 合并项目集
     *
     * @param first
     * @param second
     * @return
     */
    private static List<LRItem> mergeList(List<LRItem> first, List<LRItem> second) {
        second.stream().filter(aSecond ->
                !isInIt(first, aSecond)).forEach(first::add);
        return first;
    }

    /**
     * 判断子项目是否相等
     *
     * @param first
     * @param second
     * @return
     */
    private static boolean isListEqual(List<LRItem> first, List<LRItem> second) {
        if (first.size() != second.size())
            return false;
        for (LRItem aFirst : first) {
            if (!isInIt(second, aFirst))
                return false;
        }
        return true;
    }

    /**
     * 查找对应项目集
     *
     * @param list
     * @param map
     * @return
     */
    private static int findList(List<LRItem> list, List<List<LRItem>> map) {
        for (int i = 0; i < map.size(); i++) {
            if (isListEqual(list, map.get(i)))
                return i;
        }
        return -1;
    }

    /**
     * 初始化
     */
    public static void initTable() {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                table[i][j] = new TableItem();
                table[i][j].type = 0;
                table[i][j].where = "";
            }
        }
    }

    /**
     * 构造table
     *
     * @return
     */
    public static boolean getTable() {
        initTable();
        for (int i = 0; i < links.size(); i++) {
            Link link = links.get(i);

            TableItem tableItem = table[link.start][FrontDefault.getSymPostion(link.ch)];
            tableItem.type = -2;// 移进
            tableItem.where = FrontDefault.fixedWords.contains(link.ch) ?
                    "" + link.to : link.to + "";
        }

        for (int i = 0; i < LRList.size(); i++) {
            // 这是每一个的项目集
            List<LRItem> list = LRList.get(i);
            // j 是每个项目集
            for (int j = 0; j < list.size(); j++) {
                LRItem lrItem = list.get(j);
                // 每一个搜索符
                for (String item : lrItem.fst) {
                    // 判断在末尾
                    if (lrItem.id == FrontDefault.getPointString(lrItem.num).length()) {
                        //  规约
                        TableItem tableItem = table[i][FrontDefault.getSymPostion(item)];
                        tableItem.type = -1;
                        // 加入当前产生式的序号
                        tableItem.where = "" + lrItem.num;

                        // acc
                        if (item.equals(FrontDefault.endWord) && lrItem.num == 0) {
                            tableItem.where = "acc";
                        }
                    }
                }
            }
        }
        return true;
    }

    public static void printTable() {
        System.out.println("LR(1)分析表:    ");
        System.out.println("状态 :                ACTION");
        int fix = FrontDefault.fixedWords.size();
        int unfix = FrontDefault.unfixedWords.size();
        for (int i = 0; i < fix + unfix; i++) {
            String index = i < fix
                    ? FrontDefault.fixedWords.get(i)
                    : FrontDefault.unfixedWords.get(i - fix);
            System.out.print("      " + index);
        }

        for (int i = 0; i < LRList.size(); i++) {
            System.out.print("\n" + i + "      ");
            for (int j = 0; j < fix + unfix; j++) {
                TableItem item = table[i][j];
                if (item.type == 0) {
                    System.out.print("      ");
                }
                if (item.type == -1) {
                    if (item.where.equals("acc")) {
                        System.out.print("acc" + "      ");
                    } else {
                        System.out.print("r" + item.where + "      ");
                    }
                } else if (item.type == -2) {
                    System.out.print("s" + item.where + "      ");
                }
            }
        }
    }

    /**
     * 打印项目集
     */
    public static void printLRList() {
        int index = 0;
        for (List<LRItem> list : LRList) {
            System.out.println("项目集: " + index);
            index++;
            for (LRItem item : list) {
                System.out.println("产生式: ");
                printPointInExpression(FrontDefault.printExpression(item.num), item.id);
                System.out.println("搜索符 ");
                item.fst.forEach(System.out::print);
                System.out.print("\n");
            }
        }
    }

    private static void printPointInExpression(String exp, int pos) {
        int pox = pos == -1 ? 0 : pos;

        String left = exp.substring(0, pox + 3);
        String right = exp.substring(pox + 3);
        System.out.println(left + " . " + right);
    }

    /**
     * 判断链接是否存在
     *
     * @param link
     * @return
     */
    private static boolean isTheLineIn(Link link) {
        for (Link lin : links) {
            if (lin.start == link.start && lin.to == link.to
                    && Objects.equals(lin.ch, link.ch)) {
                return true;
            }
        }
        return false;
    }

    public static boolean analyze(String ty) {
        System.out.println();
        Stack<Integer> stateStack = new Stack<>();
        Stack<String> symStack = new Stack<>();

        int count = 0;
        stateStack.push(0);
        symStack.push(FrontDefault.endWord);

        for (int i = 0; ; ) {
            int cur = stateStack.pop();
            // 空白去死

            String ch = String.valueOf(ty.charAt(i));
            TableItem item = table[cur][FrontDefault.getSymPostion(ch)];

            if (item.type == 0)
                return false;
            // 移进
            if (item.type == -2) {
                if (item.where.equals("acc")) {
                    System.out.println("acc !!!");
                }

                symStack.push(ch);
                System.out.println("移进 !!!");
                int newState = Integer.parseInt(item.where);
                stateStack.push(newState);
                i++;
            } else if (item.type == -1) {
                // 规约
                System.out.println("规约 !!!");

            }
        }
    }
}
