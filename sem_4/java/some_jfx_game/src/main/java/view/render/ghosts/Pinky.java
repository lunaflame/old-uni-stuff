package render.ghosts;

import entities.BaseGhost;
import javafx.scene.paint.Color;
import render.GhostRenderer;

public class Pinky extends GhostRenderer {
	public Pinky(BaseGhost ghost) {
		super(ghost);
		setColor(Color.PINK);
	}
}
