package bfk.ops.IncrDecr;

import bfk.BFContext;
import bfk.BaseOp;

public class Decr implements BaseOp {

	public void Execute(BFContext ctx) {
		ctx.DecrByte();
		ctx.print("-");
	}
	
}