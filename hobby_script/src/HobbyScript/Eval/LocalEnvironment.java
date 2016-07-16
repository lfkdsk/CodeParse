package HobbyScript.Eval;

import java.util.HashMap;

/**
 * Created by liufengkai on 16/7/16.
 */
public class LocalEnvironment implements EnvironmentCallBack {
    protected HashMap<String, Object> values;

    protected EnvironmentCallBack parentEnv;

    public LocalEnvironment() {
        this(null);
    }

    public LocalEnvironment(EnvironmentCallBack parentEnv) {
        this.parentEnv = parentEnv;
        this.values = new HashMap<>();
    }

    public void setParentEnv(EnvironmentCallBack parentEnv) {
        this.parentEnv = parentEnv;
    }


    @Override
    public void put(String name, Object value) {
        this.values.put(name, value);
    }

    @Override
    public Object get(String name) {
        Object value = values.get(name);

        if (value == null && parentEnv != null) {
            return parentEnv.get(name);
        } else {
            return value;
        }
    }
}
