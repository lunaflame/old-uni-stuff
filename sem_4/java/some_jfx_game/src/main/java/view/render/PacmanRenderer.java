package render;

import entities.Pacman;
import javafx.scene.paint.Color;

public class PacmanRenderer extends EntityRenderer {
	public PacmanRenderer(Pacman pac) {
		super(pac);
	}
	

	@Override
	public void render(BaseRenderer ren) {
		var ctx = ren.getCtx();
		calcRenderBox(ren);
		
		ctx.setFill(Color.YELLOW);
		ctx.fillOval(renderBox[0], renderBox[1], renderBox[2], renderBox[2]);
	}
}
