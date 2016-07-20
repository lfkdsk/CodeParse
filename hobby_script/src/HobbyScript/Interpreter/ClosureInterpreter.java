package HobbyScript.Interpreter;

import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Exception.ParseException;
import HobbyScript.Parser.ClosureParser;

/**
 * Created by liufengkai on 16/7/17.
 */
public class ClosureInterpreter extends FunctionInterpreter{
    public static void main(String[] args) throws ParseException {
        run(new ClosureParser(), new LocalEnvironment());
    }
}
