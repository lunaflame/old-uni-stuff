package level;

import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Properties;
import java.util.Scanner;

import javafx.geometry.Point2D;

public class LevelParser {
	private String cfgPath = "parser_config.txt";

	private LevelParser() throws IOException {
		InputStream inStr = this.getClass().getClassLoader().getResourceAsStream(cfgPath);
		if (inStr == null) {
			throw new IOException("Couldn't open the level parser config @ " + cfgPath);
		}
	
		charToLevelObject.load(inStr);
	}
	
	static private LevelParser lvp = null;
	static public LevelParser get() {
		if (lvp == null) {
			try {
				lvp = new LevelParser();
			} catch (IOException ex) {
				System.out.println("failed to get level parser, idk " + ex.getMessage());
				System.exit(-1); // TODO: proper handling
			}
		}
		return lvp;
	}
	
	private Class<?> getClassByName(String name) {
		if (classLkup.containsKey(name)) { return classLkup.get(name); }
		
		Class<?> opClass;
		try {
			opClass = Class.forName(name);
			classLkup.put(name, opClass);
		} catch (ClassNotFoundException e) {
			return null;
		}
		
		return opClass;
	}
	
	private Constructor<?> getCtorByCode(char code) {
		String strCode = Character.toString(code);
		String cName = charToLevelObject.getProperty(strCode);
		
		if (cName == null) {
			return null;
		}
		
		/*if (charToLevelObject.get(cName) == null) {
			System.out.println("no obj for " + cName);
			return null;
		}*/
		
		Class<?> ctor = getClassByName(cName);
		if (ctor == null) { return null; }
		//.newInstance(null);

		try {
			return ctor.getConstructor(); //BaseLevel.class);
		} catch (NoSuchMethodException e) {
			System.out.println("no matching ctor " + code);
			return null;
		} catch (SecurityException e) {
			e.printStackTrace(); // wat
		}
		return null;
	}
	
	private void parseIntoLv(String dat, BaseLevel lv) {
		if (dat == null) { throw new NullPointerException(); }
		
		int y = 0;
		
		/*for (var val : charToLevelObject.values()) {
			System.out.println(val);
		}*/
	
		try (	Scanner scanner = new Scanner(dat)
			) {
			
			while (scanner.hasNextLine()) {
			  String line = scanner.nextLine();
			  int n = line.length();
			  
			  for (int x = 0; x < n; x++) {
				  char code = line.charAt(x);
				  var ctor = getCtorByCode(code);
				  if (ctor == null) { continue; }
					
				  BaseLevelUnit obj = (BaseLevelUnit)ctor.newInstance(); //ret);

				  obj.onParse(new Point2D(x, y));
				  obj.addToLevel(lv);
			  }
			  y++;
			}
			
		} catch(Exception ex) {
			System.out.println("Exception thrown during parsing!");
			ex.printStackTrace();
			// throw ex;
		}
		
	}

	public BaseLevel parse(String dat) {
		BaseLevel ret = new BaseLevel();
		parseIntoLv(dat, ret);
		return ret;
	}
	
	public void parseInto(String dat, BaseLevel lv) {
		parseIntoLv(dat, lv);
	}
	
	private static HashMap<String, Class<?>> classLkup = new HashMap<> ();
	private Properties charToLevelObject = new Properties(); 
}