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

    /**
     * Logger打印
     *
     * @param msg 打印message
     */
    public static void logInfo(String msg) {
        Logger.i(msg);
    }

    /**
     * 求数组长度
     *
     * @param objects 数组
     * @return 返回长度
     */
    public static int length(Object[] objects) {
        return objects.length;
    }

    /**
     * 普通日志打印
     *
     * @param msg message
     */
    public static void println(String msg) {
        System.out.println(msg);
    }

    /**
     * 创建Array数组
     *
     * @param integer 大小
     * @return 数组大小
     */
    public static Object[] createArray(Integer integer) {
        Object[] array = new Object[integer];
        for (int i = 0; i < integer; i++) {
            array[i] = 0;
        }
        return array;
    }

    public EnvironmentCallBack env(EnvironmentCallBack env) {
        addNativeFunction(env, "logInfo", NativeList.class, String.class);
        addNativeFunction(env, "println", NativeList.class, String.class);
        addNativeFunction(env, "length", NativeList.class, Object[].class);
        addNativeFunction(env, "createArray", NativeList.class, Integer.class);
        return env;
    }

}
