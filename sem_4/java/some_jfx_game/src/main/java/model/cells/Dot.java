package cells;

import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import render.LevelRenderer;

public class Dot extends BaseCell {

	public Dot() {
		setCanCollide(false);
	}
	
	public void render(LevelRenderer ren) {
		GraphicsContext ctx = ren.getCtx();
		
		calcRenderBox(ren);
		
		double fr = 0.2;
		
		ctx.setFill(Color.WHITESMOKE);
		double sz = ren.scale(fr);
		double sc = ren.scale(0.5f);
				
		ctx.fillOval(renderBox[0] + sc - sz/2, renderBox[1] + sc - sz/2,
				sz, sz);
	}
	
	public void eat() {
		getLevel().removeCell(this);
		
		if (getLevel().getCellsOfType(Dot.class).size() == 0) {
			getLevel().doWin();
		}
		
		getLevel().getGame().addScore(250);
		setLevel(null);
	}
}
