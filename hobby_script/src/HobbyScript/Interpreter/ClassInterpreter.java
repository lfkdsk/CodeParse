package HobbyScript.Interpreter;

import HobbyScript.Eval.Env.LocalEnvironment;
import HobbyScript.Exception.ParseException;
import HobbyScript.Native.NativeList;
import HobbyScript.Parser.ClassParser;

import java.io.FileNotFoundException;

/**
 * Created by liufengkai on 16/7/21.
 */
public class ClassInterpreter extends NativeInterpreter {
    public static void main(String[] args) throws FileNotFoundException, ParseException {
        run("ClassTestExtend", new ClassParser(), new NativeList().env(new LocalEnvironment()));
    }
}
