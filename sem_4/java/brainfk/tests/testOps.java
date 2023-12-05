import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestMethodOrder;
import org.junit.jupiter.api.MethodOrderer.OrderAnnotation;

import bfk.BFContext;
import bfk.BFFactory;
import bfk.BFParser;
import junit.framework.TestCase;


public class testOps extends TestCase  {

	private BFParser parser;
	private BFContext ctx;
	private BFFactory fac;

	@BeforeEach
	@Override
    protected void setUp() throws Exception {
		BFFactory.SetConfigPath("test_factory.cfg");
		fac = BFFactory.Get();
		assertDoesNotThrow(() -> fac.Reload());
	
        parser = new BFParser();
        ctx = parser.GetContext();
    }
	
	@AfterEach
	@Override
    protected void tearDown() throws Exception {
        // ?
		parser = null;
		ctx = null;
    }
	
	private void runCode(String code) {
		parser.SetBfCode(code);
		assertDoesNotThrow(() -> parser.Parse());
		assertDoesNotThrow(() -> parser.Execute());
	}

	@Test
	void basic() {
		runCode(">++<-");
		
		assertEquals(ctx.GetPtr(), 0);
		assertEquals(ctx.GetByte(), (char)255);
	
		ctx.IncrPtr();
		assertEquals(ctx.GetPtr(), 1);
		assertEquals(ctx.GetByte(), 2);
	}
	
	@Test
	void loops() {
		// test incorrect sequences
		parser.SetBfCode("[++++");
		assertThrows(Exception.class, () -> parser.Parse());
		
		parser.SetBfCode("++++]");
		assertThrows(Exception.class, () -> parser.Parse());
		
		// test basic looping
		runCode("+++[>+++<-]");
		
		assertEquals(0, ctx.GetPtr());
		assertEquals(0, ctx.GetByte()); // looped until 0
		
		ctx.IncrPtr();
		assertEquals(9, ctx.GetByte()); // incremented 3 x3 times
		
		// test nested loops
		String code = "+++" +	// [0] = 3; 
		"[>+++++" +	// [1] = 5;
			"[>+++<-]" + // for each in [1], add 3 to [2]
		"<-]"; // repeat [0] times
		
		runCode(code);
		// result: 3 * 5 * 3 = 45 in [2]
	
		assertEquals(0, ctx.GetPtr());
		assertEquals(0, ctx.GetByte());
		
		ctx.IncrPtr();
		assertEquals(0, ctx.GetByte());

		ctx.IncrPtr();
		assertEquals(45, ctx.GetByte());
		
		code = "[<]-	Comment cool#"; // loops shouldn't run and should just skip when cell == 0
		runCode(code);
		
		assertEquals(0, ctx.GetPtr());
		assertEquals((char)255, ctx.GetByte());
	}
}
