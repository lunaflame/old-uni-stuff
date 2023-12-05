package bfk.ops.IncrDecr;

import bfk.BFContext;
import bfk.BaseOp;

public class Incr implements BaseOp {
	public void Execute(BFContext ctx) {
		ctx.IncrByte();
		ctx.print("+");
	}
}