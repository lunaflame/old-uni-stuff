package entities.ghosts;

import entities.BaseGhost;
import entities.Pacman;
import javafx.geometry.Point2D;
import javafx.scene.paint.Color;

public class Inky extends BaseGhost {
	public Inky() {
		super();
		setColor(Color.CYAN);
		setRenderer(new render.ghosts.Inky(this));
	}

	@Override
	public void tick(double ft) {
		super.tick(ft);
		
		if (inHouse) {
			if (getLevel().getGame().getPlayTime() > 10) {
				targetHouseExit();
			}
		} else {
			Pacman pac = findPacman();
			Point2D pos = pac.getCellPos();
			Point2D vecDir = pac.getDirection().vecDir;
			
			Point2D moveTo = pos.add(vecDir);
			
			setTargetCell(moveTo);
		}
	
		doMove(ft);
	};
	
	public static float invSqrt(float x) {
	    float xhalf = 0.5f * x;
	    int i = Float.floatToIntBits(x); 	// evil floating point bit level hacking
	    i = 0x5f3759df - (i >> 1); 			// what the fuck?
	    x = Float.intBitsToFloat(i);
	    x *= (1.5f - xhalf * x * x);		// 1st iteration
//		x  *= x * (1.5f - (xhalf * x * x)); // 2nd iteration, this can be removed
	    return x;
	}
	
	protected Color col = Color.CYAN;
}
