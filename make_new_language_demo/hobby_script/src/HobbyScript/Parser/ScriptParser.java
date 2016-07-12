package HobbyScript.Parser;

import HobbyScript.Utils.BnfParser;

import java.util.HashSet;

/**
 * 构建语法树
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class ScriptParser {
    HashSet<String> reserved = new HashSet<>();

    BnfParser.Operators operators = new BnfParser.Operators();

    BnfParser expr0 = BnfParser.rule();

//    BnfParser primary = BnfParser.rule()
}
