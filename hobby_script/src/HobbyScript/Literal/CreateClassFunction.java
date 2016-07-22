package HobbyScript.Literal;

import HobbyScript.Ast.BlockStmnt;
import HobbyScript.Ast.ParameterList;
import HobbyScript.Eval.Env.EnvironmentCallBack;

/**
 * Created by liufengkai on 16/7/22.
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
