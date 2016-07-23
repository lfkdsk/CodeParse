package HobbyScript.Literal;

import HobbyScript.Ast.BlockStmnt;
import HobbyScript.Ast.ParameterList;
import HobbyScript.Eval.Env.EnvironmentCallBack;

/**
 * 类构造函数
 * 返回时会携带类实例
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/22.
 */
public class CreateClassFunction extends ClassFunction {
    private HobbyObject classObject;

    public CreateClassFunction(ParameterList parameters, BlockStmnt body, EnvironmentCallBack env) {
        super(parameters, body, env);
    }


    public CreateClassFunction(Function function, HobbyObject object) {
        super(function.parameters, function.body, function.env);
        this.classObject = object;
    }

    public HobbyObject getClassObject() {
        return classObject;
    }

    public void setClassObject(HobbyObject classObject) {
        this.classObject = classObject;
    }
}
