package bfk.ops.IO;

import bfk.BFContext;
import bfk.BaseOp;

public class Output implements BaseOp {
	public void Execute(BFContext ctx) {
		ctx.println("outputting " + ctx.GetByte());
		System.out.print((char)ctx.GetByte());
	}
}