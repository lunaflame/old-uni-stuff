package render.ghosts;

import entities.BaseGhost;
import javafx.scene.paint.Color;
import render.GhostRenderer;


public class Blinky extends GhostRenderer {
	public Blinky(BaseGhost ghost) {
		super(ghost);
		setColor(Color.RED);
	}
}
