package HobbyScript.Eval;

/**
 * Created by liufengkai on 16/7/14.
 */
public interface EnvironmentCallBack {
    void put(String name, Object value);

    Object get(String name);
}
