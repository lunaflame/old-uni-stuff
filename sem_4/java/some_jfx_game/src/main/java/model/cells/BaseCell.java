package cells;

import entities.BaseEntity;
import javafx.geometry.Point2D;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import level.BaseLevel;
import level.BaseLevelUnit;
import render.LevelRenderer;


// TODO: temporarily not abstract
public class BaseCell implements BaseLevelUnit {
	public BaseCell() {
		// TODO Auto-generated constructor stub
	}
	
	@Override
	public void addToLevel(BaseLevel lv) {
		lv.addCell(this);
		setLevel(lv);
	}
	
	@Override
	public void onParse(Point2D pos) {
		setPos(pos);
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
	
	// allocating an array of 4 doubles per-render is nonsense (but so is this)
	// I LOVE JAVA I LOVE OBJECTS I LOVE ALLOCATIONS
	public static double[] renderBox = new double[4];
	
	protected void calcRenderBox(LevelRenderer ren) {
		double x = ren.getPx(getPos().getX()), y = ren.getPx(getPos().getY());
		double sc = ren.scale(1);
		
		renderBox[0] = x; renderBox[1] = y; renderBox[2] = sc; renderBox[3] = sc;
	}
	
	public void render(LevelRenderer ren) {
		GraphicsContext ctx = ren.getCtx();
		
		calcRenderBox(ren);
		
		double pulse = 0.5 + 0.5 * Math.sin((double)System.currentTimeMillis() / 1000 * Math.PI * 2);
		
		Color col = new Color( (150 + pulse * 75) / 255, 50. / 255, 50. / 255, 1);
		
		ctx.setFill(col);
		ctx.fillRect(renderBox[0], renderBox[1], renderBox[2], renderBox[3]);
	}
	
	/**
	 * @return whether this cell will generally block movement
	 */
	public boolean canCollide() {
		return canCollide;
	}
	
	/**
	 * @return whether this cell will block movement for a specific entity
	 */
	public boolean canCollide(BaseEntity ent) {
		return canCollide();
	}

	/**
	 * @param canCollide whether this cell should block movement
	 */
	public void setCanCollide(boolean canCollide) {
		this.canCollide = canCollide;
	}


	public BaseLevel getLevel() {
		return lv;
	}

	public void setLevel(BaseLevel lv) {
		this.lv = lv;
	}

	protected boolean canCollide = false;
	private Point2D pos = null;
	private BaseLevel lv = null;
}
