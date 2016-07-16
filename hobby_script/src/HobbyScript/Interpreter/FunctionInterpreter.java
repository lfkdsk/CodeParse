package HobbyScript.Interpreter;

import HobbyScript.Eval.LocalEnvironment;
import HobbyScript.Exception.ParseException;
import HobbyScript.Parser.FunctionParser;

/**
 * Created by liufengkai on 16/7/16.
 */
public class FunctionInterpreter extends ScriptInterpreter {
    public static void main(String[] args) throws ParseException {
        run(new FunctionParser(), new LocalEnvironment());
    }
}
