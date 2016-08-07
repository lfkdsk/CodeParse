package HobbyScript.Compile;

/**
 * Created by liufengkai on 16/8/7.
 */
public interface CompileCallBack {
    String compile(CodeLine line, int start, int end);
}
