package level;

import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;

import entities.BaseEntity;
import javafx.animation.AnimationTimer;

public class GameState {
	public interface TickListener {
		default void onTick() {};
		default void onTick(double ft) { onTick(); }
		
		default void onRender() {};
		default void onRender(double ft) { onRender(); }
	}
	
	public void onLoop(TickListener ls) { listeners.add(ls); }
	public void doLevelTick(double ft) {
		for (TickListener lis : listeners) {
			if (!dead) {
				lis.onTick(ft);
			}
		}
		
		for (TickListener lis : listeners) {
			lis.onRender(ft);
		}
	}
	
	public void start() {
		if (tmr != null) {
			// unpause instead of creating a new timer
			tmr.start();
			tmr.unpause();
			return;
		}

		tmr = new TickTimer();
		tmr.start();
		globTmr.start();
	}

	public void pause() {
		tmr.pause();
	}
	
	public void addScore(int add) {
		score += add;
	}
	
	public int getScore() {
		return score;
	}
	
	public double getPlayTime() {
		return playTime;
	}

	public void setPlayTime(double playTime) {
		this.playTime = playTime;
	}
	
	public int getLives() {
		return lives;
	}
	
	public void setLives(int lives) {
		this.lives = lives;
	}

	public boolean isDead() {
		return dead;
	}
	
	public void setDead(boolean dead) {
		this.dead = dead;
	}
	
	public void die() {
		setDead(true);
		
		Timer t = new Timer();
        t.schedule(new DieTask(this), 3000);
	}
	
	public BaseLevel getLevel() {
		return lv;
	}
	public void setLevel(BaseLevel lv) {
		this.lv = lv;
	}
	
	public void doGameOver() {
		// ? save to leaderboards?
		setGameOver(true);
	}

	public boolean isGameOver() {
		return gameOver;
	}
	public void setGameOver(boolean gameOver) {
		this.gameOver = gameOver;
	}

	public double getGlobTime() {
		double sex = globTime / 1e9;
		return sex;
	}

	public double getFrametime() {
		double ft = this.ft / 1e9;
		return ft;
	}

	private class DieTask extends TimerTask {
		private GameState gm;
		
		DieTask(GameState gm) { this.gm = gm; }
		
        public void run() {
        	if (getLives() > 0) {
        		setLives(getLives() - 1);
        		
	            for (Object oent : gm.getLevel().getEnts()) {
	            	BaseEntity ent = (BaseEntity)oent;
	            	ent.reset();
	            }
	            
	            setPlayTime(0);
	            setDead(false);
        	} else {
        		doGameOver();
        	}
        }
    }
	
	// why do you have to use this for rendering anyways
	// instead of a sane binding to the jfx event loop?
	private TickTimer tmr = null;

	private class TickTimer extends AnimationTimer {
		private long lastFrame = 0;

		public void pause() {
			lastFrame = 0;
			stop();
		}

		public void unpause() {
			lastFrame = 0;
		}

		@Override
		public void handle(long ns) {
			if (lastFrame == 0) {
				lastFrame = ns;
				return;
			}

			long passed = ns - lastFrame;
			double secs = passed / 1e9;
			setPlayTime(getPlayTime() + secs);
			lastFrame = ns;

			doLevelTick(secs);
		}
	}
	
	private AnimationTimer globTmr = new AnimationTimer() {
		private long startWhen = 0;
		private long lastFrame = 0;
		
		@Override
		public void handle(long arg0) {
			if (startWhen == 0) {
				startWhen = arg0;
				ft = 0;
			} else {
				ft = arg0 - lastFrame;
			}
			
			lastFrame = arg0;
			globTime = (arg0 - startWhen);
		}
	};
	
	private long globTime = 0;
	private long ft = 0;
	
	private HashSet<TickListener> listeners = new HashSet<> ();
	private double playTime = 0;
	
	private int score = 0;
	private int lives = 1;
	private boolean dead = false;
	private boolean gameOver = false;
	private BaseLevel lv;
}
