package bfk;

import java.util.ArrayList;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BFContext extends BFOpInterface {
	
	public static final int DefaultCap = 65536; 
	private Logger log = LogManager.getLogger("BFContext");
	
	public BFContext() {
		
	}
	
	public void SetUp() {
		dataIdx = 0;
		opIdx = 0;
		
		data.clear();
		data.ensureCapacity(DefaultCap);

		for (int i = 0; i < DefaultCap; i++) {
			data.add( (char)0 );
		}
		data.trimToSize();
		log.debug("New execution context created");
	}
	
	public void ExecuteOps() {
		if (ops == null) {
			throw new IllegalArgumentException("No ops to run");
		}
		
		SetUp();
		
		log.info("Executing " + ops.size() + " ops");
		for (; opIdx < ops.size();) {
			BaseOp op = ops.get(opIdx);
			//System.out.println("Executing " + op.getClass().getName());
			op.Execute(this);
			
			opIdx += shifted ? 0 : 1;
			shifted = false;
		}
		log.info("Execution finished");
	}
	
	// i could do a weird locking mechanism but EHHHH
	public void SetOps(ArrayList<BaseOp> newOps) { ops = newOps; }
	public char GetByte() {
		assertByte();
		return data.get(dataIdx);
	}
	public int GetPtr() { return dataIdx; }

	// questionable
	private void assertByte() {
		if (data.size() <= dataIdx) {
			for (int i = 0; i <= (dataIdx - data.size() + 1); i++) {
				data.add((char)0);
			}
		}
	}
	public void IncrByte() {
		assertByte();
		data.set( dataIdx, (char)( Math.floorMod(data.get(dataIdx) + 1, 256) ) );
	}

	public void DecrByte() {
		assertByte();
		// System.out.println("decr: " + data.get(dataIdx) + ", " + Math.floorMod(data.get(dataIdx) - 1, 256));
		data.set( dataIdx, (char)( Math.floorMod(data.get(dataIdx) - 1, 256) ) );
	}
	
	public void SetByte(char to) {
		assertByte();
		data.set( dataIdx, to );
	}
	// Data pointers; used to shift the currently active cell
	// (ie: "<", ">")
	public void IncrPtr() { dataIdx++; }
	public void DecrPtr() {
		if (dataIdx == 0) {
			log.error("Attempted to move past the left memory edge, get owned");

			throw new IllegalStateException("Brainfunk doesn't specify what should happen once you " +
					"move past the left memory tape edge, so screw you jerk heres an exception");
		}
		
		dataIdx--;
	}
	
	public void SetOpPtr(int ptr) {
		opIdx = ptr;
		shifted = true; // hack: whenever an op changes op ptr, we don't increment 1 during execution
						// makes more sense that you'd set op ptr to (ptr) rather than (ptr - 1)
	}
	
	public void SetDataPtr(int ptr) {
		dataIdx = ptr;
	}
	
	public void println(String str) {
		//System.out.println(str);
	}
	
	public void print(String str) {
		//System.out.print(str);
	}

	public int GetOpPtr() { return opIdx; }

	public ArrayList<Character> DebugGetData() { return data; }

	private int dataIdx = 0;
	private int opIdx = 0;
	private int defaultCap = 32; 
	private boolean shifted = false;

	private ArrayList<Character> data = new ArrayList<Character> (defaultCap);
	private ArrayList<BaseOp> ops = null;
}
