package cells;

import entities.BaseEntity;
import entities.BaseGhost;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import render.LevelRenderer;

public class HouseEntrance extends BaseCell {
	public HouseEntrance() {
		super();
		setCanCollide(true);
	}

	protected void calcRenderBox(LevelRenderer ren) {
		double x = ren.getPx(getPos().getX()), y = ren.getPx(getPos().getY());
	
		double one = ren.scale(1);
		double w = one, h = ren.scale(0.2);

		renderBox[0] = x; renderBox[1] = y; // + one / 2 - h / 2;
		renderBox[2] = w; renderBox[3] = h;
	}
	
	@Override
	public void render(LevelRenderer ren) {
		GraphicsContext ctx = ren.getCtx();
		
		calcRenderBox(ren);

		Color col = new Color(205. / 255, 145. / 255, 120. / 255, 1);
		
		ctx.setFill(col);
		ctx.fillRect(renderBox[0], renderBox[1], renderBox[2], renderBox[3]);
	}
	
	@Override
	public boolean canCollide(BaseEntity ent) {
		if (ent instanceof BaseGhost) {
			BaseGhost gh = (BaseGhost) ent;
			if (gh.getInHouse()) {
				return false; // only collide when exiting the house
			}
		}
		return true;
	}
	
	@Override
	public boolean canCollide() {
		System.out.println("cancollide called...wtf!?");
		return true;
	}
}
