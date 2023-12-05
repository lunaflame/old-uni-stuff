package bfk;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BFParser extends BFOpInterface {
	private Logger log = LogManager.getLogger("BFParser");

	public String ReadFromFile(String filePath) {
		String content = null;

		try {
			content = Files.readString(Paths.get(filePath));
		} catch (IOException ex) {
			// ??
			log.error("Exception while opening the input code file @ " + filePath);
			System.out.println("Exception while opening input code file.");
		}

		SetBfCode(content);
		return content;
	}
	
	public void Parse() throws Exception {
		if (GetBfCode() == null) {
			log.error("Attemped to parse without any code loaded.");
			throw new IllegalArgumentException("Can't parse code without any code being loaded!");
		}
		
		parseSuccess = false;
		
		BFFactory fac = BFFactory.Get();
		ops.clear();
		ClearCustomData();
	
		String bfCode = GetBfCode();
		int n = bfCode.length(); // mfw
		
		try {
			for (cursor = 0; cursor < n; cursor++) {
				char code = bfCode.charAt(cursor);
				if (!fac.CodeHasOp(code)) {
					continue;
				}
				
				BaseOp op = fac.GetOpByCode(code);
				op.Parse(this);
				ops.add(op);
			}
			
			Iterator<BaseOp> iter = ops.iterator();
		    while (iter.hasNext()) {
		    	BaseOp op = iter.next();
		    	op.FinishParse(this);
		    }

			parseSuccess = true;
		} catch(Exception ex) {
			log.error("Exception thrown while parsing ops: " + ex.getMessage());
			throw ex;
		}
	}
	
	public void Execute() {
		if (!parseSuccess) {
			log.error("Attemped to execute code without parsing first.");
			throw new IllegalStateException("Can't execute code without parsing!");
		}
		
		BFContext ctx = GetContext();

		ctx.SetOps(ops);
		ctx.SetCustomDataSet(GetCustomDataSet());
		ctx.ExecuteOps();
	}
	
	public BFContext GetContext() {
		if (ctx == null) {
			ctx = new BFContext();
		}
		
		return ctx;
	}
	public String GetBfCode() {
		return bfCode;
	}

	public void SetBfCode(String bfCode) {
		this.bfCode = bfCode;
	}
	
	public int GetCursor() {
		return cursor;
	}
	
	public int GetOpsTop() {
		return ops.size();
	}

	/*public void SetCursor(int cursor) {
		this.cursor = cursor;
	}*/

	private String bfCode = null;
	private int cursor = 0;
	private boolean parseSuccess = false;
	private ArrayList<BaseOp> ops = new ArrayList<BaseOp>();
	private BFContext ctx = null; // dont make a context until we need it
}
