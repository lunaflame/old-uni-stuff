package cells;

import javafx.scene.canvas.GraphicsContext;
import javafx.scene.paint.Color;
import render.LevelRenderer;

public class Wall extends BaseCell {

	public Wall() {
		super();
		setCanCollide(true);
	}
	
	public void render(LevelRenderer ren) {
		GraphicsContext ctx = ren.getCtx();
		
		calcRenderBox(ren);

		Color col = new Color( 50. / 255, 150. / 255, 250. / 255, 1);
		
		ctx.setFill(col);
		ctx.fillRect(renderBox[0], renderBox[1], renderBox[2], renderBox[3]);
	}
}
