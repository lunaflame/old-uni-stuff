package bfk.ops.Shift;

import bfk.BFContext;
import bfk.BaseOp;

public class ShiftLeft implements BaseOp {
	public void Execute(BFContext ctx) {
		ctx.DecrPtr();
		ctx.print("<");
	}
}