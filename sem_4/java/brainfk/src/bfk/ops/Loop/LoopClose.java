package bfk.ops.Loop;

import bfk.BFContext;
import bfk.BFParser;

public class LoopClose implements BaseLoop {
	public void Execute(BFContext ctx) {
		int opener = lookupPair(ctx, ctx.GetOpPtr());
	
		// this isn't supposed to happen, ever; this stuff must be caught at parse time
		if (opener == -1) {
			throw new IllegalStateException("Closing bracket didn't find an opening during runtime...? " + ctx.GetOpPtr() + " -> ?");
		}
		
		ctx.print("\n] ");
	
		if (ctx.GetByte() > 0) {
			ctx.SetOpPtr(opener);
		}
	}
	
	public void Parse(BFParser prs) {
		// during parse stage, make sure the sequence is correct
		var stk = ensureStack(prs);

		Integer lastOpener = stk.pollLast();

		if (lastOpener == null) {
			throw new IllegalArgumentException("No opening bracket (\"[\") for the closing bracket (\"]\") !");
		}
		
		makePair(prs, prs.GetOpsTop(), lastOpener);
	}
}