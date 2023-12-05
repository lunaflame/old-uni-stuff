package render;

import cells.BaseCell;
import javafx.scene.paint.Color;
import level.BaseLevel;
import screens.GameScreen;

public class LevelRenderer extends BaseRenderer {
	public LevelRenderer(GameScreen scr) {
		super(scr);
	}

	public void doRender() {
		BaseLevel lv = scr.getLevel();
		
		double w = getW(), h = getH();
		
		// a single unit's rendered scale
		double scale = Math.min(w / lv.getW(), h / lv.getH());
		this.scale = Math.floor(scale);
		
		var ctx = getCtx();
		ctx.setFill(Color.BLACK);
		ctx.fillRect(0, 0, w, h);
		
		var cells = lv.getCells();

		for (Object ocell : cells.values()) {
			BaseCell cell = (BaseCell)ocell; // :sob:
			cell.render(this);
		}
	}
	
}
