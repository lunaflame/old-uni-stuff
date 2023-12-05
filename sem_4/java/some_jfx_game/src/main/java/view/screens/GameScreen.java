package screens;

import java.util.ArrayList;

import entities.Pacman;
import game.PacmanController;
import javafx.scene.Group;
import javafx.scene.Parent;
import javafx.scene.canvas.Canvas;
import javafx.stage.Stage;
import level.BaseLevel;
import level.GameState;
import level.GameState.TickListener;
import render.BaseRenderer;
import render.HUDRenderer;
import render.BaseEntityRenderer;
import render.LevelRenderer;

// boys we gamin

public class GameScreen extends BaseScreen {
	
	public GameScreen() {

	}
	
	private void makeRenderers() {
		if (!rens.isEmpty()) { return; } // already made renderers on previous show or something
		
		rens.add(new LevelRenderer(this));
		rens.add(new BaseEntityRenderer(this));
		rens.add(new HUDRenderer(this));
		
		getGame().onLoop(new TickListener() {
			public void onRender() {
				getCanvas().getGraphicsContext2D().clearRect(0, 0, getCanvas().getWidth(), getCanvas().getHeight());
				for (BaseRenderer ren : rens) {
					ren.doRender();
				}
			}
		});
	}
	
	@Override
	protected Parent onShow() {
		if (lv == null) { throw new NullPointerException(); }
		Group root = new Group();
		Canvas canv = new Canvas();
		
		Stage stg = this.getWindow().getStage();
		
		canv.widthProperty().bind(stg.getScene().widthProperty());
        canv.heightProperty().bind(stg.getScene().heightProperty());
        
		root.getChildren().add(canv);
		
		// set up everything pacman related - renderers, controllers, etc.
		PacmanController click = new PacmanController(this);
		var pacs = lv.getEntsOfType(Pacman.class);
		
		// this isn't ever supposed to run as the check should be elsewhere, but better safe than sorry ig
		if (pacs.size() == 0) { throw new IllegalStateException("i love my pacman where is my pacman TrollDespair"); }
		
		click.setPacman((Pacman)pacs.get(0));
		// click.setPacman();
		
		makeRenderers();
		getGame().start();
	
		this.setCanvas(canv);
	
		return root;
	}
	

	public BaseLevel getLevel() {
		return lv;
	}

	public void setLevel(BaseLevel lv) {
		this.lv = lv;
		this.setGame(lv.getGame());
	}

	public Canvas getCanvas() {
		return canv;
	}

	private void setCanvas(Canvas canv) {
		this.canv = canv;
	}

	public GameState getGame() {
		return game;
	}

	public void setGame(GameState game) {
		this.game = game;
	}

	private Canvas canv = null;
	private ArrayList<BaseRenderer> rens = new ArrayList<> (4);
	private BaseLevel lv = null;
	private GameState game = null;
}
