package bfk.ops.Shift;

import bfk.BFContext;
import bfk.BaseOp;

public class ShiftRight implements BaseOp {
	public void Execute(BFContext ctx) {
		ctx.IncrPtr();
		ctx.print(">");
	}
}