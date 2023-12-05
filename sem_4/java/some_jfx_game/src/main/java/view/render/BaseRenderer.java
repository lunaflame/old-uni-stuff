package render;

import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import screens.GameScreen;

public abstract class BaseRenderer {
	public BaseRenderer(GameScreen scr) {
		if (scr == null) { throw new NullPointerException(); }
		
		this.scr = scr;
	}
	
	public abstract void doRender();
	
	public double getScale() { return scale; }
	
	// useful for remapping units (game field position) to pixels
	// ie if a wall is at (3, 3) and each unit is 16px then you should render
	// a 16x16 sprite at position (48, 48)px 									(...NOW!)
	public double getPx(double u) { return scale(u); }
	public double scale(double u) { return Math.floor(u * scale); }

	// ^ btw the flooring is there because decimal positions are super wonky
	
	public double getW() { return scr.getCanvas().getWidth(); }
	public double getH() { return scr.getCanvas().getHeight(); }
	public Canvas getCanv() { return scr.getCanvas(); }
	public GraphicsContext getCtx() { return getCanv().getGraphicsContext2D(); }
	
	protected double scale = 0;
	protected GameScreen scr;
}
