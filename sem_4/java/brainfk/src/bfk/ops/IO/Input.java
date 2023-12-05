package bfk.ops.IO;

import java.io.IOException;

import bfk.BFContext;
import bfk.BaseOp;

public class Input implements BaseOp {
	private byte[] readBuf = new byte[32];

	public void Execute(BFContext ctx) {
		// If we didn't or couldn't read something, we ignore it (ie dont modify the memory cell)
		// If we did, slap the byte into the current memory cell
	
		int out;
		try {
			out = System.in.read();
		} catch (IOException e) {
			return;
		}
		
		int available = 0;
		try {
			available = System.in.available();
		} catch (IOException e) {
			return;
		}
	
		// 13 = \r, 10 == \n
		if (out == -1 || (available < 2 && (out == '\r' || out == '\n')) ) { return; }
		
		ctx.SetByte((char)out);
	}
}