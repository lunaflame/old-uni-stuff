package level;

public class Collisions {
	
	public static class CollisionResult {
		public boolean collided; // whether we collided at all
		public double collFr; // collision frac (how far into the movement did we collide
		public double nx, ny; // collision normals
	}
	
	public void setBox1(double x, double y, double w, double h) {
		box1.x = x;
		box1.y = y;
		box1.w = w;
		box1.h = h;
	}
	
	public void setBox2(double x, double y, double w, double h) {
		box2.x = x;
		box2.y = y;
		box2.w = w;
		box2.h = h;
	}
	
	
	private static CollisionResult res = new CollisionResult();
	
	public static CollisionResult sweptCollide(double vx, double vy) {
		Box b1 = box1, b2 = box2;
		
		double xInvEntry, yInvEntry; 
		double xInvExit, yInvExit; 

		// find the distance between the objects on the near and far sides for both x and y 
		if (vx > 0.0f) { 
		  xInvEntry = b2.x - (b1.x + b1.w);  
		  xInvExit = (b2.x + b2.w) - b1.x;
		} else { 
		  xInvEntry = (b2.x + b2.w) - b1.x;  
		  xInvExit = b2.x - (b1.x + b1.w);  
		} 

		if (vy > 0.0f) { 
		  yInvEntry = b2.y - (b1.y + b1.h);  
		  yInvExit = (b2.y + b2.h) - b1.y;  
		} else { 
		  yInvEntry = (b2.y + b2.h) - b1.y;  
		  yInvExit = b2.y - (b1.y + b1.h);  
		}
		
		double xEntry, yEntry; 
		double xExit, yExit; 

		if (vx == 0.0f)  { 
		  xEntry = Double.NEGATIVE_INFINITY; 
		  xExit = Double.POSITIVE_INFINITY; 
		} else  { 
		  xEntry = xInvEntry / vx; 
		  xExit = xInvExit / vx; 
		} 

		if (vy == 0.0f)  { 
			yEntry = Double.NEGATIVE_INFINITY; 
			yExit = Double.POSITIVE_INFINITY; 
		} else  { 
			yEntry = yInvEntry / vy; 
			yExit = yInvExit / vy; 
		} 
		
		double entryFr = Math.max(xEntry, yEntry);
		double exitFr = Math.max(xExit, yExit);
		
//		System.out.println(" b1: " + b1.x + ", " + b1.y);
//		System.out.println(" b2: " + b2.x + ", " + b2.y);
//		System.out.println(" coll: " + xInvEntry + ", " + vx + ": " + xEntry);
		
		if (xEntry < 0 && yEntry < 0) { res.collided = false; return res; }
		if (yEntry > 1 || xEntry > 1) { res.collided = false; return res; }
		if (entryFr > exitFr) { res.collided = false; return res; }

		res.collided = true;
		res.collFr = entryFr;

		if (xEntry > yEntry) {
			// collision happened on the X axis
			res.ny = 0;

			if (xInvEntry < 0) {
				res.nx = 1; 
			} else {
				res.nx = -1;
			}
		} else {
			// collision on the Y axis
			res.nx = 0;

			if (yInvEntry < 0) {
				res.ny = 1; 
			} else {
				res.ny = -1;
			}
		}
		
		return res;
	}
	
	private static class Box {
		public double x, y, w, h;
	}
	
	private static Box box1 = new Box();
	private static Box box2 = new Box();
}
