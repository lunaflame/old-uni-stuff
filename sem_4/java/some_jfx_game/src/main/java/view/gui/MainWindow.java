package gui;

import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;
import screens.BaseScreen;

public class MainWindow {

	public MainWindow() {
		
	}
	public MainWindow(Stage stg) {
		setStage(stg);
	}
	
	
	public void showScreen(BaseScreen screen) {
		if (stage == null) { throw new NullPointerException(); }
		if (screen == null) { throw new NullPointerException(); }
		
		if (curScreen != null) {
			curScreen.doHide(stage);
		}
		if (stage.getScene() == null) {
			stage.setScene(new Scene(new VBox()));
		}
		
		curScreen = screen;
		//Scene newScene = screen.doShow(stage);
		Parent newPar = screen.doShow(this);
		if (newPar != null) {
			//stage.setScene(newScene);
			
			stage.getScene().setRoot(newPar); // wtf
			stage.show();
		}
	}

	/**
	 * @return the stage
	 */
	public Stage getStage() {
		return stage;
	}

	/**
	 * @param stage the stage to set
	 */
	public void setStage(Stage stage) {
		this.stage = stage;
	}

	private Stage stage = null;
	private BaseScreen curScreen = null;
}
