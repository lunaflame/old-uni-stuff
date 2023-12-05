package entities.ghosts;

import entities.BaseGhost;
import entities.Pacman;
import javafx.geometry.Point2D;
import javafx.scene.paint.Color;

// 	... His targeting scheme attempts to move him to the place where Pac-Man is going,
// 		instead of where he currently is. Pinky's target tile in Chase mode is
// 		determined by looking at Pac-Man's current position and orientation, and
// 		selecting the location four tiles straight ahead of Pac-Man.

public class Pinky extends BaseGhost {
	public Pinky() {
		super();
		setColor(Color.PINK);
		setRenderer(new render.ghosts.Pinky(this));
	}
	
	private static double lookAhead = 4;

	@Override
	public void tick(double ft) {
		super.tick(ft);
		if (inHouse) {
			if (getLevel().getGame().getPlayTime() > 20) {
				targetHouseExit();
			}
		} else {
			Pacman pac = findPacman();
			Point2D pos = pac.getCellPos();
			Point2D vecDir = pac.getDirection().vecDir.multiply(lookAhead);
			
			Point2D moveTo = pos.add(vecDir);
			
			setTargetCell(moveTo);
		}

		doMove(ft);
	};
	
	protected Color col = Color.PINK;
}
