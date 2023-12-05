package entities;

import cells.BaseCell;
import cells.HouseEntrance;
import javafx.geometry.Point2D;
import javafx.scene.paint.Color;
import level.BaseLevel;

// should this be like an interface or something?
public class BaseGhost extends BaseMovableEntity {
	public enum Behavior {
		SCATTER,
		FRIGHTENED,
		CHASE,
	}
	
	private Direction lastDir = Direction.NONE;
	private Point2D lastPos;
	private Direction lastChoice = Direction.NONE;
	private boolean help = false;

	protected void setDebug(boolean b) {
		help = b;
	}

	protected Direction calculateMove() {
		Point2D curCell = getCellPos();
		if (curCell.equals(lastPos)) { // only allowed to change directions once per cell
			return lastChoice;
		}
		
		lastPos = curCell;
		Point2D target = getTargetCell();

		Direction choice = Direction.NONE;
		double minDist = Double.MAX_VALUE;
		BaseLevel lv = getLevel();
		
		// System.out.println("cur pos:" + curCell);
		
		Direction opposite = Direction.NONE;
	
		for (Direction dir : Direction.values()) {
			if (dir == Direction.NONE) { continue; }
		
			// if the direction is the exact opposite, ignore
			Point2D sum = dir.vecDir.add(lastDir.vecDir);
						
			if (sum.getX() == 0 && sum.getY() == 0) {
				opposite = dir;
				continue;
			}

			Point2D nextCell = curCell.add(dir.vecDir);
			// check if it's better for us to move there than what we already have
			double dist = nextCell.distance(target);
			if (dist > minDist) { continue; }
			
			// check if it's even possible to move there (ie no collidable cells)
			boolean canMove = false;
			BaseCell cell = lv.getCell(nextCell);
		
			// no cell = can move
			if (cell == null) 					{ canMove = true; }
			// uncollideable cell = can move
			if (!canMove && !cell.canCollide(this)) { canMove = true; }
				
			if (help) {
				System.out.println(dir + " @ " + nextCell + "[dist" + dist + "/" + minDist + "] : " + cell + " / " + canMove);
			}
		
			if (canMove) {
				minDist = dist;
				choice = dir;
			}
		}
			
		if (choice == Direction.NONE) { // we can't move anywhere... try to back out
			return opposite;
		}
		
		lastChoice = choice;
		if (help) {
			System.out.println("calculated choice:" + choice + ", " + lastDir);
		}

		return choice;
	}
	
	@Override
	public void setDirection(Direction dir) {
		super.setDirection(dir);
		// track what direction we went in so we can disallow going there next time
		lastDir = dir;
		
		if (help) {
			System.out.println("set direction yes? " + dir);
			new Exception().printStackTrace();
		}
	}

	@Override
	protected Direction getWantDirection() {
		return calculateMove();
	}

	protected void targetHouseExit() {
		HouseEntrance bruh = (HouseEntrance) getLevel().getCellsOfType(HouseEntrance.class).get(0);

		if (bruh == null) {
			System.out.println("no entrance!?");
			inHouse = false;
			return;
		}
		
		if (getCellPos().equals(bruh.getPos().add(Direction.UP.vecDir))) {
			inHouse = false;
		}
	
		setTargetCell(bruh.getPos().add(Direction.UP.vecDir));
		
		if (getDirection() == Direction.NONE) {
			setDirection(getWantDirection());
		}
	}

	// convenience function
	protected Pacman findPacman() {
		return (Pacman) getLevel().getEntsOfType(Pacman.class).get(0);
	}

	public Point2D getTargetCell() {
		return target;
	}

	public void setTargetCell(Point2D target) {
		this.target = target;
	}
	
	public Color getColor() {
		return col;
	}

	public void setColor(Color col) {
		this.col = col;
	}
	
	public boolean getInHouse() {
		return inHouse;
	}
	
	@Override
	public void reset() {
		super.reset();
		System.out.println("reset ghost... in house now");
		inHouse = true;
		target = new Point2D(0, 0);
		setDirection(Direction.NONE);
	}
	
	@Override
	public void tick(double ft) {
		super.tick(ft);
		var pac = findPacman();

		if (pac.getCellPos().equals(this.getCellPos())) {
			System.out.println("DEATH");
			pac.die();
		}
	}
	
	private Point2D target = new Point2D(0, 0);
	protected boolean inHouse = true;
	private Color col = Color.RED;
}
