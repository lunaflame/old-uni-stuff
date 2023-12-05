package render.ghosts;

import entities.BaseGhost;
import javafx.scene.paint.Color;
import render.GhostRenderer;

public class Inky extends GhostRenderer {
	public Inky(BaseGhost ghost) {
		super(ghost);
		setColor(Color.CYAN);
	}
}
