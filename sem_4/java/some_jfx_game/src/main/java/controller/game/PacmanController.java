package game;

import entities.BaseMovableEntity.Direction;
import entities.Pacman;
import javafx.scene.Scene;
import screens.GameScreen;

public class PacmanController {

	public PacmanController(GameScreen scr) {
		Scene sc = scr.getWindow().getStage().getScene(); // wtf

		sc.setOnKeyPressed(e -> {
			if (getPacman() == null) { return; }
			Pacman waka = getPacman();
			
			Direction dir = null;
			switch (e.getCode()) {
				case UP: dir = Direction.UP; break;
				case DOWN: dir = Direction.DOWN; break;
				case LEFT: dir = Direction.LEFT; break;
				case RIGHT: dir = Direction.RIGHT; break;
			default:
				break;
			}
			
			if (dir == null) { return; }

			waka.setWantDirection(dir);
		});
	}
	
	public void setPacman(Pacman pc) { waka = pc; }
	public Pacman getPacman() { return waka; }
	
	private Pacman waka = null;
}
