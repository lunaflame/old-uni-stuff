package bfk.ops.Loop;

import java.util.ArrayDeque;
import java.util.HashMap;

import bfk.BFContext;
import bfk.BFOpInterface;

public interface BaseLoop extends bfk.BaseOp {

	public void Execute(BFContext ctx);

	@SuppressWarnings("unchecked") // owned
	default ArrayDeque<Integer> ensureStack(BFOpInterface ctx) {
		Object cur = ctx.GetCustomData("LoopStack"); 
		if (cur == null) {
			ArrayDeque<Integer> newQ = new ArrayDeque<Integer> ();
			ctx.SetCustomData("LoopStack", newQ);
			return newQ;
		}
		
		return (ArrayDeque<Integer>)cur;
	}
	
	@SuppressWarnings("unchecked")
	private HashMap<Integer, Integer> getLkup(BFOpInterface ctx) {
		Object cur = ctx.GetCustomData("LoopLkup"); 
		if (cur == null) {
			HashMap<Integer, Integer> newHt = new HashMap<Integer, Integer> ();
			ctx.SetCustomData("LoopLkup", newHt);
			return newHt;
		}
		
		return (HashMap<Integer, Integer>)cur;	
	}

	default int lookupPair(BFOpInterface ctx, int pair1) {
		return getLkup(ctx).getOrDefault(pair1, -1);
	}
	
	default void makePair(BFOpInterface ctx, int pair1, int pair2) {
		getLkup(ctx).put(pair1, pair2);
		getLkup(ctx).put(pair2, pair1);
	}
}
