package entities.ghosts;

import entities.BaseGhost;
import entities.Pacman;
import javafx.scene.paint.Color;

// ... Blinky's target tile in Chase mode is defined as Pac-Man's current tile.
public class Blinky extends BaseGhost {
	public Blinky() {
		super();
		setColor(Color.RED);
		setRenderer(new render.ghosts.Blinky(this));
	}

	@Override
	public void tick(double ft) {
		super.tick(ft);
		if (inHouse) {
			targetHouseExit();
		} else {	
			Pacman pac = findPacman();
			setTargetCell(pac.getCellPos());
		}
		
		doMove(ft);
	};

}
