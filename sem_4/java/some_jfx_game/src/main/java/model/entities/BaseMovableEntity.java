package entities;

import cells.BaseCell;
import javafx.geometry.Point2D;
import level.BaseLevel;
import level.Collisions.CollisionResult;

public abstract class BaseMovableEntity extends BaseEntity {
	public enum Direction {
		
		NONE	(new Point2D(0, 0)),
		UP		(new Point2D(0, -1)),
		RIGHT	(new Point2D(1, 0)),
		DOWN	(new Point2D(0, 1)),
		LEFT	(new Point2D(-1, 0));
		
		public final Point2D vecDir;
		private Direction(Point2D dir) {
			vecDir = dir;
		}
	}
	
	public static double GlobalSpeedMult = 3;
	
	public BaseMovableEntity() {
		super();
	}
	
	/**
	 * @return the velocity (how many cells the entity will move by in 1 second)
	 */
	public Point2D getVelocity() {
		return velocity;
	}
	/**
	 * @param velocity the velocity to set (how many cells the entity will move by in 1 second)
	 */
	public void setVelocity(Point2D velocity) {
		this.velocity = velocity;
	}
	
	public void setVelocity(double x, double y) {
		// WHY CAN I NOT REUSE THE SAME POINT2D REEEEEEEEEEEEEE
		velocity = new Point2D(x, y);
	}
	
	protected void onChangedCell(Point2D op, Point2D np) {
		double newX = np.getX(), newY = np.getY();
		double oX = newX, oY = newY; // only setpos if newX/newY were changed

		BaseLevel lv = getLevel();
		
		// offset from the edge to teleport to
		// is this a giant hack? yes
		double EPS = 1e-2;
		boolean allowDir = true;
	
		if (np.getX() < -1) { // wrap around on the X axis
			newX = lv.getW() - EPS;
		} else if (np.getX() > lv.getW()) {
			newX = -1 + EPS;
		}
		
		if (np.getY() < -1) { // wrap around on the Y axis
			newY = lv.getH() - EPS;
		} else if (np.getY() > lv.getH()) {
			newY = -1 + EPS;
		}
		
		allowDir = (newX == oX && newY == oY); // only allow switching dirs if we didn't wrap around
	
		if (allowDir && getWantDirection() != getDirection()) {
			// get the cell in the direction in which we'll go
			double cellX = Math.floor(newX + 0.5), cellY = Math.floor(newY + 0.5);
			Point2D wantDir = getWantDirection().vecDir;
			
			// is it solid? if yes, we ignore the direction swap and just keep going
			Point2D checkPos = lv.snapPos(wantDir.add(np));
			BaseCell cell = lv.getCell(checkPos);
			
			boolean canMove = (cell == null || !cell.canCollide(this));

			if (canMove) {
				newX = cellX; newY = cellY;
				setDirection(getWantDirection());
			}
		}
		
		if (oX != newX || newY != oY) {
			setPos(new Point2D(newX, newY));
		}
	}
	
	protected void onCollided(Point2D op, Point2D np) {
		if (getWantDirection() != getDirection()) {
			setDirection(getWantDirection());
		}
	}
	
	public void doMove(double ft) {
		Point2D old = getPos();
		Point2D vel = getVelocity();
		
		CollisionResult res = getLevel().Collision.collideWallsSwept(this, vel.getX() * ft, vel.getY() * ft);
		
		if (res.collided) {
			double canMove = res.collFr;
			if (canMove > 0) {
				setPos(new Point2D(old.getX() + vel.getX() * ft * canMove, old.getY() + vel.getY() * ft * canMove));
			}
		} else {
			setPos(new Point2D(old.getX() + vel.getX() * ft, old.getY() + vel.getY() * ft));
		}
		
		Point2D newPos = getPos();
	
		if ( Math.floor(old.getX()) != Math.floor(newPos.getX()) ||
				Math.floor(old.getY()) != Math.floor(newPos.getY())) {
			onChangedCell(old, newPos);
		} else if (res.collided) {
			onCollided(old, newPos);
		}
	}
	
	public Point2D dirToVec(Direction dir) {
		return dir.vecDir;
	}
	
	public void setDirection(Direction dir) {
		this.curDir = dir;
		setVelocity(dirToVec(dir).multiply(GlobalSpeedMult));
	}
	
	public Direction getDirection() {
		return this.curDir;
	}
	
	public void setWantDirection(Direction dir) {
		this.wantDir = dir;

		if (getDirection() == Direction.NONE) {
			setDirection(dir);
		} else {
			// if the direction is the exact opposite, change direction immediately
			Point2D help = dir.vecDir.add(getDirection().vecDir);
			
			if (help.getX() == 0 && help.getY() == 0) {
				setDirection(dir);
			}
		}
	}
	
	protected Direction getWantDirection() {
		return this.wantDir;
	}
	
	private Direction wantDir = Direction.RIGHT;
	
	private Direction curDir = Direction.NONE;
	private Point2D velocity = new Point2D(0, 0);
}
