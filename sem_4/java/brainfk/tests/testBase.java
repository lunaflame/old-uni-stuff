import static org.junit.jupiter.api.Assertions.*;

import java.io.IOException;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.MethodOrderer.OrderAnnotation;
import org.junit.jupiter.api.Order;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.TestMethodOrder;

import bfk.BFContext;
import bfk.BFFactory;
import bfk.BFOpInterface;
import bfk.BFParser;
import junit.framework.TestCase;

@TestMethodOrder(OrderAnnotation.class)
public class testBase extends TestCase {
	private BFParser parser;
	private BFContext ctx;

	@BeforeEach
	@Override
    protected void setUp() throws Exception {
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
	
	@Test
	@Order(1)
	void init() {
		BFOpInterface interf = new BFOpInterface();
		interf.SetCustomData("SomeKey", 10);
		assertEquals(interf.GetCustomData("SomeKey"), 10);
		assertNotNull(interf.GetCustomDataSet());
	
		// test out super basic stuff; parsing, executing, etc.
		BFFactory fac = BFFactory.Get();
	
		BFFactory.SetConfigPath(null);
		assertThrows(IllegalArgumentException.class, () -> fac.Reload());
		
		BFFactory.SetConfigPath("\\i_do_not_exist");
		assertThrows(IOException.class, () -> fac.Reload());
	
		BFFactory.SetConfigPath("test_factory.cfg");
		assertDoesNotThrow(() -> fac.Reload());
	
		assertNull(fac.GetOpByCode((char) 0));
		assertNull(fac.GetOpByCode('/')); // the class for it is defined in the config but doesn't exist
		assertNull(fac.GetOpByCode('|')); // the class exists but is some random thing (a java bool in this case)
		assertNull(fac.GetOpByCode('|')); // test caching missing ops
		assertNotNull(fac.GetOpByCode('+'));

		assertThrows(IllegalArgumentException.class, () -> parser.Parse()); // no code loaded; cannot parse
		assertThrows(IllegalStateException.class, () -> parser.Execute());
			
		String expectedCode = ">++<-.";
	  	parser.SetBfCode(expectedCode);
	  	assertEquals(expectedCode, parser.GetBfCode());
	  	
	  	parser.ReadFromFile("testBase_code_example.txt");
	  	assertEquals(expectedCode, parser.GetBfCode());
	  	
	  	// parsing & executing is done in the ops tests
	  	
	  	assertNotNull(ctx);
	  	
	  	ctx.SetUp(); // allocate memory and setup default data
	  
	  	assertEquals(0, ctx.GetPtr());
	  	assertEquals((byte)0, (byte)ctx.GetByte());
	  	
	  	// 0-th pointer = 0
	  	// 0 - 1 = 255
	  	ctx.DecrByte();
	  	assertEquals((byte)255, (byte)ctx.GetByte());
	  	
	  	// 255 + 2 = 1
	  	ctx.IncrByte(); ctx.IncrByte();
	  	assertEquals((byte)1, (byte)ctx.GetByte());
	  	
	  	// 1-th pointer = 0
	  	ctx.IncrPtr();
	  	assertEquals((byte)0, ctx.GetByte());
	  	
	  	ctx.SetOpPtr(0);
	  	assertEquals(0, ctx.GetOpPtr());
	  	
	  	ctx.DecrPtr();
	  	assertThrows(IllegalStateException.class, () -> ctx.DecrPtr()); // moving past the left edge of memory throws
	}
	
	@Test
	void growBytes() {
		// test out the ability of the context to grow its' memory 
		for (int i = 0; i < BFContext.DefaultCap * 4; i++) {
			ctx.SetByte( (char) i );
			ctx.IncrPtr();
		}

		ctx.SetDataPtr(0);
	
		for (int i = 0; i < BFContext.DefaultCap * 4; i++) {
			assertEquals(ctx.GetByte(), (char)(i));
			ctx.IncrPtr();
		}
	}
}
