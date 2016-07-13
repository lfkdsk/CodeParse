package HobbyScript.Parser;

import HobbyScript.Parser.AstList;
import HobbyScript.Parser.AstNode;

import java.util.List;

/**
 * 代码块
 *
 * @author liufengkai
 *         Created by liufengkai on 16/7/12.
 */
public class BlockStmnt extends AstList {

    public BlockStmnt(List<AstNode> children) {
        super(children);
    }

}
