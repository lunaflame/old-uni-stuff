
import java.util.NoSuchElementException;
import java.util.Timer;
import java.util.TimerTask;

import gui.MainWindow;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.stage.Screen;
import javafx.stage.Stage;
import screens.InitLoading;
import screens.MainMenu;

public class Main extends Application {
	private MainWindow wnd;
	
	// java fx is mega cheeks; thank u stackoverflow
	// https://stackoverflow.com/a/61688876
	public static Screen getScreenForStage(Stage stage) {
		System.out.println(stage.getX() + "," + stage.getY());
	
	    for (Screen screen : Screen.getScreensForRectangle(stage.getX(), stage.getY(), 1., 1.)) {
	        return screen;
	    }
	    throw new NoSuchElementException("Cannot determine screen for stage.");
	}
	
    @Override
    public void start(Stage stage) {
    	Platform.setImplicitExit(true);
        stage.setOnCloseRequest((ae) -> {
            Platform.exit();
            System.exit(0);
        });

        MainWindow wnd = new MainWindow(stage);
        this.wnd = wnd;
       
        stage.setMaximized(true);
        
        stage.setMinWidth(256);
        stage.setMinHeight(stage.getMinWidth() * 16 / 9); // height > width
       
        var loading = new InitLoading();
        loading.setLoadAmount(6);
        wnd.showScreen(loading);
        
        Timer t = new Timer();
        t.schedule(new LoadTask(loading), 0, 50);
    }

	public static void main(String[] args) {
        launch();
    }
	
	public void showMainMenu() {
		MainMenu mm = new MainMenu();
		this.wnd.showScreen(mm);
	}
	
	private class LoadTask extends TimerTask {
		private InitLoading ld;
		
		LoadTask(InitLoading ld) { this.ld = ld; }
        public void run() {
            // load something i guess? currently a placeholder
        	ld.incrLoad();
        	
        	if (ld.getLoadAmount() == ld.getLoaded()) {
        		// this syncs the java thread with the jfx thread
        		// (eugh)
        		Platform.runLater(new Runnable() {
        	        @Override
        	        public void run() {
        	        	showMainMenu();
        	        }
        	    });
        	}
        }
    }
}