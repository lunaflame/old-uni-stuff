package entities;

import cells.Dot;
import level.BaseLevel;
import render.PacmanRenderer;

// waka waka mfer

public class Pacman extends BaseMovableEntity {

	public Pacman() {
		super();
		setVelocity(dirToVec(Direction.NONE)); // lol
		setRenderer(new PacmanRenderer(this));
	}
	
	@Override
	public void tick(double ft) {
		doMove(ft);
		BaseLevel lv = getLevel();
		if (lv.getCell(getCellPos()) instanceof Dot) {
			Dot dot = (Dot) lv.getCell(getCellPos());
			dot.eat();
		}
	}
	
	public void die() {
		getLevel().getGame().die();
	}
	
}
