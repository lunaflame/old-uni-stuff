package screens;

import gui.MainWindow;
import javafx.scene.Parent;
import javafx.stage.Stage;

public abstract class BaseScreen {

	public Parent doShow(MainWindow wnd) {
		setWindow(wnd);
		Parent sc = onShow();
		// setScene(sc);
		return sc;
	}
	
	public void doHide(Stage stage) {
		boolean shouldHide = onHide(stage); // questionable use from the stage methinks
		
		if (shouldHide) {
			// getScene().
			// ?????
		}
	}
	
	protected Parent onShow() { return null; }
	protected boolean onHide(Stage stage) { return true; }
	
	/**
	 * @return the par
	 */
	public Parent getParent() {
		return par;
	}

	/**
	 * @param par the par to set
	 */
	public void setParent(Parent par) {
		this.par = par;
	}

	/**
	 * @return the wnd
	 */
	public MainWindow getWindow() {
		return wnd;
	}

	/**
	 * @param wnd the wnd to set
	 */
	public void setWindow(MainWindow wnd) {
		this.wnd = wnd;
	}

	private Parent par = null;
	private MainWindow wnd = null;
}
