package HobbyScript.Native;

import HobbyScript.Eval.Env.EnvironmentCallBack;
import HobbyScript.Exception.HobbyException;
import HobbyScript.Literal.NaiveFunction;
import HobbyScript.Utils.logger.Logger;

import java.lang.reflect.Method;

/**
 * 原生函数对应列表
 * Created by liufengkai on 16/7/20.
 */
public class NativeList {

    /**
     * 添加原声函数
     *
     * @param env        环境 fuck
     * @param nativeName 函数名
     * @param clazz      需要反射拿到的类
     * @param params     参数
     */
    public static void addNativeFunction(EnvironmentCallBack env,
                                         String nativeName,
                                         Class<?> clazz,
                                         Class<?>... params) {
        Method method;

        try {
            method = clazz.getMethod(nativeName, params);
        } catch (NoSuchMethodException e) {
            throw new HobbyException("can not found native function :" + nativeName);
        }

        env.put(nativeName, new NaiveFunction(nativeName, method));
    }

    public static void logInfo(String msg) {
        Logger.init();
        Logger.i(msg);
    }

    public EnvironmentCallBack env(EnvironmentCallBack env) {
        addNativeFunction(env, "logInfo", NativeList.class, String.class);

        return env;
    }

}
