package level;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;

import cells.BaseCell;
import entities.BaseEntity;
import javafx.geometry.Point2D;
import javafx.geometry.Rectangle2D;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import level.GameState.TickListener;

public class BaseLevel {
	public Map<Point2D, ? super BaseCell> getCells() {
		return Collections.unmodifiableMap(cells);
	}

	public double getW() { return size.getX(); }
	public double getH() { return size.getY(); }
	public Point2D getSize() { return size; }
	public boolean isReady() { return isSetup; }
	
	public Point2D snapPos(Point2D cur) {
		return new Point2D(Math.floor(cur.getX() + 0.5), Math.floor(cur.getY() + 0.5));
	}

	public BaseLevel() {
		// TODO: make a constructor that allows initial values for size for faster init
		ents = new ArrayList<> ();
		cells = new Hashtable<> ();
	}
	
	public List<? super BaseEntity> getEnts() {
		return Collections.unmodifiableList(ents);
	}
	
	public ArrayList<? super BaseEntity> getEntsOfType(Class<? extends BaseEntity> typ) {
		var ec = entByClass.get(typ);
		
		if (ec == null) {
			ec = new ArrayList<> (16);
			entByClass.put(typ, ec);
		}
		
		return ec;
	}
	
	public ArrayList<? super BaseCell> getCellsOfType(Class<? extends BaseCell> typ) {
		var ec = cellByClass.get(typ);
		
		if (ec == null) {
			ec = new ArrayList<> (16);
			cellByClass.put(typ, ec);
		}
		
		return ec;
	}
	
	public void addCell(BaseCell cell) {
		if (cell.getPos() == null) {
			throw new IllegalArgumentException("Cell has no position set up!");
		}
	
		cells.put(cell.getPos(), cell);

		var key = (Class<? extends BaseCell>) cell.getClass(); // my god
		var ec = cellByClass.get(key);
		
		if (ec == null) {
			ec = new ArrayList<> (16);
			cellByClass.put(key, ec);
		}
		
		ec.add(cell);
	}
	
	public void removeCell(BaseCell cell) {
		cells.remove(cell.getPos());
		var key = (Class<? extends BaseCell>) cell.getClass(); // my god
		var ec = cellByClass.get(key);
		
		if (ec == null) { return; }
		ec.remove(cell);
	}

	public <T extends BaseEntity> void addEntity(T ent) {
		if (!ents.contains(ent)) { // TODO: maybe a list is better than an array here?
			ents.add(ent);
			var key = (Class<? extends BaseEntity>) ent.getClass();
			var ec = entByClass.get(key);

			if (ec == null) {
				ec = new ArrayList<> (16);
				entByClass.put(key, ec);
			}

			ec.add(ent);
		}
	}

	public BaseCell getCell(Point2D pos) {
		return (BaseCell) cells.get(pos);
	}
	
	public void finish() {
		if (game == null) {
			throw new IllegalStateException("Can't finish a level without a set GameState.");
		}
		
		if (isSetup) {
			throw new IllegalStateException("Can't finish a level twice.");
		}
	
		isSetup = true;
		double maxX = 0, maxY = 0;

		for (Object ocell : cells.values()) {
			BaseCell cell = (BaseCell)ocell;
			maxX = Math.ceil( Math.max(maxX, cell.getPos().getX() + 1) );
			maxY = Math.ceil( Math.max(maxY, cell.getPos().getY() + 1) );
		}
		
		size = new Point2D(maxX, maxY);
		
		this.getGame().onLoop(new TickListener() {
			public void onTick(double ft) {
				for (Object oent : ents) {
					BaseEntity ent = (BaseEntity)oent; // THIS SUCKS
					ent.tick(ft);
				}
			}
		});
	}

	public LevelCollider Collision = new LevelCollider();
	
	public class LevelCollider extends Collisions {
		private ArrayList<BaseCell> colliders = new ArrayList<>(16); // we will reuse this array
		private static double EPSILON = 1e-9; // wtf

		private void fillColliders(BaseEntity ent, Rectangle2D hitbox, double vx, double vy) {
			double minx = hitbox.getMinX() + (vx < 0 ? vx : EPSILON);
			double maxx = hitbox.getMaxX() + (vx > 0 ? vx : -EPSILON);
			
			double miny = hitbox.getMinY() + (vy < 0 ? vy : EPSILON);
			double maxy = hitbox.getMaxY() + (vy > 0 ? vy : -EPSILON);
			
			double sX = Math.floor(minx); // left-most cell it can possibly touch
			double eX = Math.floor(maxx);

			double sY = Math.floor(miny);
			double eY = Math.floor(maxy);

			int possibleCells = (int) ((eX - sX + 1) * (eY - sY + 1));
			colliders.clear(); // TODO: would it be faster to slap a null at [sz] so it'd break there?
			colliders.ensureCapacity(possibleCells);
			
			// System.out.println("col calc: " + sX + ", " + eX + "; " + sY + "; " + eY);
			int sz = 0;

			for (double x = sX; x <= eX; x++) {
				for (double y = sY; y <= eY; y++) {
					Point2D origin = new Point2D(x, y);
					// System.out.println("have cell at " + x + ", " + y + "? " + cells.get(origin));
					BaseCell cell = (BaseCell) cells.get(origin); // !!! TODO: object creation, thanks immutability

					if (cell != null && cell.canCollide(ent)) {
						colliders.add(sz, cell); 
						sz++;
						// System.out.println("  collider @ (" + x + ", " + y + ")");
					}
				}
			}
		}
		
		public static CollisionResult swept = new CollisionResult();
		
		public CollisionResult collideWallsSwept(BaseEntity ent, double vx, double vy) {
			Rectangle2D box = ent.getHitbox();
			fillColliders(ent, box, vx, vy);
			setBox1(box.getMinX(), box.getMinY(), box.getWidth(), box.getHeight());

			swept.collided = false;
			swept.collFr = 1;
			
			//System.out.println("trying out collisions " +
			//		box.getMinX() + ", " + box.getMinY() + ", " + box.getMaxX() + ", " + box.getMaxY());

			for (BaseCell cell : colliders) {
				if (cell == null) { break; }
				Point2D pt = cell.getPos();
				setBox2(pt.getX(), pt.getY(), 1, 1);
				// System.out.println("    Swept colliding against " + pt.getX() + ", " + pt.getY());
				CollisionResult col = Collisions.sweptCollide(vx, vy);
				if (!col.collided) { continue; }
				
				if (swept.collFr > col.collFr) {
					swept.collided = true;
					swept.collFr = col.collFr;
					swept.nx = col.nx; swept.ny = col.ny;
				}
			}
			
			
			return swept;
		}
		
		/*public boolean collideWalls(Rectangle2D hitbox) {
			fillColliders(hitbox, 0, 0);

			for (BaseCell cell : colliders) {
				if (cell == null) { break; }
				Point2D pt = cell.getPos();

				if (hitbox.intersects(pt.getX(), pt.getY(), 1, 1)) {
					return true;
				}
			}

			return false;
		}*/
	}
	
	public void doWin() {
		Alert alert = new Alert(AlertType.INFORMATION);
		alert.setTitle("SWEET VICTORY");
		alert.setHeaderText("very good very nice");
		alert.setContentText("Vi von zulul");
		alert.show();
		
		getGame().pause();
	}
	
	public GameState getGame() {
		return game;
	}

	public void setGame(GameState game) {
		this.game = game;
		game.setLevel(this);
	}
	
	private ArrayList<? super BaseEntity> ents = null;
	private Hashtable<Class<? extends BaseEntity>, ArrayList<? super BaseEntity>> entByClass = new Hashtable<> ();
	
	// cells have a integer position (ie (1, 0), (1, 1), etc.)
	// we can abuse this for faster collision checks (round down the
	// expected position's XY and check if theres a wall here)
	private Hashtable<Point2D, ? super BaseCell> cells = null;
	private Hashtable<Class<? extends BaseCell>, ArrayList<? super BaseCell>> cellByClass = new Hashtable<> ();
	
	// we don't need a rect; level origins are always @ 0, 0
	private Point2D size = null;
	
	private boolean isSetup = false;

	
	
	private GameState game = null;
}
