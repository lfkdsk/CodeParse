package HobbyScript.Eval.Env;

/**
 * Created by liufengkai on 16/7/14.
 */
public interface EnvironmentCallBack {
    void put(String name, Object value);

    boolean contains(String name);

    Object get(String name);
}
