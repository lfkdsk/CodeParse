package HobbyScript.Literal;

import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Eval.Env.LocalEnvironment;

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

    public Object read(String name) throws AssessException {
        return getEnvironment(name).get(name);
    }

    public void write(String name, Object value) throws AssessException {
        ((LocalEnvironment) getEnvironment(name)).putLocal(name, value);
    }

    private EnvironmentCallBack getEnvironment(String member) throws AssessException {
        EnvironmentCallBack environment = ((LocalEnvironment) env).foundEnv(member);

        if (environment == null || environment == env) {
            return env;
        }

        throw new AssessException("can not assess member: " + member);
    }

    /**
     * 无法访问异常
     */
    public static class AssessException extends Exception {
        public AssessException(String msg) {
            super(msg);
        }
    }

}
