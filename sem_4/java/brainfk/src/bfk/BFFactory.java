package bfk;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Properties;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;

public class BFFactory {
	public static void SetConfigPath(String fp) {
		cfgPath = fp;
	}

	public static BFFactory Get() {
		if (instance == null) {
			instance = new BFFactory();
		}
		
		return instance;
	}
	
	public void Reload() throws IOException {
		missingOps.clear();
		charToOp.clear();
		charToOpName.clear();
		
		if (cfgPath == null) {
			throw new IllegalArgumentException("Factory's Config path is null!");
		}
	
		readObjectFile(cfgPath);
	}
	
	public boolean CodeHasOp(char code) {
		return GetOpByCode(code) != null;
	}
	
	/**
	 * @param code
	 * @return
	 */
	public BaseOp GetOpByCode(char code) {
		var op = charToOp.get(code);
		if (op == null) {
			op = loadNewOp(code);
		}
		return op;
	}
	
	private Logger log = LogManager.getLogger("BFFactory");
	
	private BFFactory() {
	
		charToOp = new HashMap<Character, BaseOp> (256);
		charToOpName = new Properties();
		missingOps = new HashMap<Character, Boolean> (256);
		try {
			readObjectFile(cfgPath);
		} catch (IOException e) {
			log.error("Failed to read config file during init: " + cfgPath);
		}
	}
	
	/**
	 * Reads the factory config file containing conversions
	 * code in the input file <-> operation class
	 * 
	 * @param filePath - path to the factory config file
	 * @throws IOException 
	 */
	private void readObjectFile(String filePath) throws IOException {
		try {
			InputStream inStr = this.getClass().getClassLoader().getResourceAsStream(filePath);

			if (inStr == null) {
				log.error("getResourceAsStream returned null for `".concat(filePath) + "`. Missing file?");
				throw new IOException("No file: " + filePath);
			}
			
			charToOpName.load(inStr);
			log.info("Read config successfully.");
		} catch (IOException ex) {
			log.error("Failed to open factory config file: ".concat(ex.toString()));
			throw ex;
		}
	}

	/**
	 * Creates a new op instance using the data from the factory config file.
	 * 
	 * @param code - character corresponding to the required operation
	 * @return
	 */

	private BaseOp loadNewOp(char code) {
		// we already tried to get this op and failed; just bail

		if (missingOps.containsKey(code)) {
			return null;
		}
	
		String name = charToOpName.getProperty(Character.toString(code));
		if (name == null) {
			missingOps.put(code, true);
			log.info("No op for opcode: " + code);
			return null;
		}
		
		Class<?> opClass;
		try {
			opClass = Class.forName(name);
		} catch (ClassNotFoundException e) {
			missingOps.put(code, true);
			log.warn("No class for opcode ".concat(name));
			return null;
		}
		
		BaseOp op = null;
		
		try {
			op = (BaseOp)opClass.getConstructor().newInstance();
			charToOp.put(code, op);
			log.info("Instantiated op for the first time: " + name);
		} catch (Exception ex) {
			log.warn("Failed to instantiate operation ".concat(name));
		}
	
		return op;
	}
	
	
	private static BFFactory instance = null;
	private static HashMap<Character, BaseOp> charToOp = null;
	private static HashMap<Character, Boolean> missingOps = null;
	private static Properties charToOpName = null;
	private static String cfgPath = "res/factory.cfg";
}
