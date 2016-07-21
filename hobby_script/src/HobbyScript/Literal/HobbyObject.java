package HobbyScript.Literal;

import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Exception.HobbyException;

/**
 * Class类的子对象
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/21.
 */
public class HobbyObject {
    /**
     * 内层环境
     */
    protected EnvironmentCallBack env;

    public HobbyObject(EnvironmentCallBack env) {
        this.env = env;
    }

    @Override
    public String toString() {
        return "<object: " + hashCode() + " > ";
    }

    public Object read(String name) {
        return getEnvironment(name).get(name);
    }

    public void write(String name, Object value) {
        ((LocalEnvironment) getEnvironment(name)).putLocal(name, value);
    }

    private EnvironmentCallBack getEnvironment(String member) {
        EnvironmentCallBack environment = ((LocalEnvironment) env).foundEnv(member);

        if (environment == null || environment == env) {
            return env;
        } else {
            throw new HobbyException("can not assess member: " + member);
        }
    }

    /**
     * 无法访问异常
     */
    public static class AssessException extends HobbyException {
        public AssessException(String msg) {
            super(msg);
        }
    }

}
