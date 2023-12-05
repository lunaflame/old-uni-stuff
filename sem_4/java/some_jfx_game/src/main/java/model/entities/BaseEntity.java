package entities;

import javafx.geometry.Point2D;
import javafx.geometry.Rectangle2D;
import level.BaseLevel;
import level.BaseLevelUnit;
import render.EntityRenderer;

public abstract class BaseEntity implements BaseLevelUnit {
	public BaseEntity() {

	}
	
	@Override
	public void addToLevel(BaseLevel lv) {
		lv.addEntity(this);
		setLevel(lv);
	}
	
	@Override
	public void onParse(Point2D pos) {
		this.setPos(pos);
		ogPos = pos;
	}
	/**
	 * @return the position on the game field
	 */
	public Point2D getPos() {
		return pos;
	}
	/**
	 * @param pos the position to set
	 */
	public void setPos(Point2D pos) {
		this.pos = pos;
	}
	
	// hitboxes are used for collisions and such; for drawing, see calcRenderBox
	public Rectangle2D getHitbox() {
		return new Rectangle2D(pos.getX(), pos.getY(), size.getX(), size.getY());
	}
	
	// get the cell in which the center of this ent lies
	public Point2D getCellPos() {
		Point2D pos = getPos();
		return new Point2D(Math.floor(pos.getX() + size.getX() / 2), Math.floor(pos.getY() + size.getY() / 2));
	}
	
	public void reset() {
		this.setPos(ogPos);
	}
	
	public void tick(double ft) { }
	
	public BaseLevel getLevel() {
		return lv;
	}

	public void setLevel(BaseLevel lv) {
		this.lv = lv;
	}

	public EntityRenderer getRenderer() {
		return ren;
	}

	public void setRenderer(EntityRenderer ren) {
		this.ren = ren;
	}

	private Point2D pos;
	private Point2D ogPos;
	private Point2D size = new Point2D(1, 1); // do i need size? i dont think i need size
	private BaseLevel lv;
	private EntityRenderer ren;
}
