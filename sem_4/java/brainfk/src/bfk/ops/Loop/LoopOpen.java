package bfk.ops.Loop;

import java.util.ArrayDeque;
import bfk.BFContext;
import bfk.BFParser;

public class LoopOpen implements BaseLoop {
	public void Execute(BFContext ctx) {
		if (ctx.GetByte() == 0) {
			// jump to the closing bracket
			ctx.SetOpPtr(lookupPair(ctx, ctx.GetOpPtr()));
		} else {
			ctx.println("[");
		}
	}
	
	public void Parse(BFParser prs) {
		ArrayDeque<Integer> stk = ensureStack(prs);
		Integer cur = prs.GetOpsTop();
		stk.add(cur);
	}
	
	public void FinishParse(BFParser prs) {
		ArrayDeque<Integer> stk = ensureStack(prs);

		if (!stk.isEmpty()) {
			// we still have an open [ by the time we finish parsing; something went wrong...
			throw new IllegalStateException("No closing bracket (\"]\") for the opening bracket (\"[\") !");
		}
	}
}