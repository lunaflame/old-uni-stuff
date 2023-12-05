package bfk.ops;

import java.util.ArrayList;

import bfk.BFContext;
import bfk.BaseOp;

public class DebugOp implements BaseOp {

	public void Execute(BFContext ctx) {
		ArrayList<Character> ar = ctx.DebugGetData();
		
		for (int i = 0; i < 32; i++) {
			char chr = ar.get(i);
			ctx.println((ctx.GetPtr() == i ? "-> " : "    ") + i + ": " + (int)chr);
		}
	}
	
}