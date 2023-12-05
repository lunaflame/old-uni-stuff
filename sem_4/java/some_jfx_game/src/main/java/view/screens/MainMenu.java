package screens;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Timer;
import java.util.TimerTask;

import javafx.application.Platform;
import javafx.geometry.Pos;
import javafx.scene.Parent;
import javafx.scene.control.Button;
import javafx.scene.layout.VBox;
import level.GameState;
import level.TestingLevel;

public class MainMenu extends BaseMenuScreen {
	
	private String[][] buttons = {
			
		// string #1: the method name to invoke on click ( `Play` -> `doPlay(Button btn)` )
		// string #2: the label to show on the button
		{ "Play", "Play" },
		{ "Leaderboards", "Leaderboards" },
		{ "Settings", "Settings" },
		{ "Quit", "Quit" },
	};
	
	@Override
	protected Parent onShow() {
		VBox bgbx = new VBox(12);
		setPaneParent(bgbx);
		
		bgbx.setAlignment(Pos.CENTER);
		
		for (String[] data : buttons) {
			addButton(createBtn(data));
		}
		
		return bgbx;
	}
	
	public void doPlay(Button btn) {
		var me = this;
		
		GameState game = new GameState() {
			@Override
			public void doGameOver() {
				super.doGameOver();

				new Timer().schedule(new TimerTask() {
					public void run() {
						// this syncs the java thread with the jfx thread
						// (eugh)
						Platform.runLater(new Runnable() {
							@Override
							public void run() {
								getWindow().showScreen(me);
							}
						});
					}
				}, 5000);
			}
		};
		
		GameScreen gaming = new GameScreen();
		TestingLevel lv = new TestingLevel(game);
		gaming.setLevel(lv);
		
		getWindow().showScreen(gaming);
	}
	
	private Button createBtn(String[] data) {
		String methodName = data[0];
		String name = data[1];
		
		Button btn = new Button();
		btn.setPrefSize(256, 64);
		btn.setText(name);
		
		Method meth; // jesse
		try {
			meth = getClass().getMethod("do" + methodName, Button.class);
		} catch (NoSuchMethodException e) {
			btn.setText("[MISSING METHOD: " + methodName + "]");
			return btn;
		}
		
		btn.setOnMouseClicked(ev -> {
			try {
				meth.invoke(this, btn);
			} catch (IllegalAccessException | IllegalArgumentException e) {
				e.printStackTrace();
				btn.setText("[Nice error]");
			} catch (InvocationTargetException e) {
				// exception due to invoking
				e.getCause().printStackTrace();
				btn.setText("[Nice error]");
			}
		});
		
		return btn;
	}
}
