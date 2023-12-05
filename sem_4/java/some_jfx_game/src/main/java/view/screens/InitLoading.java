package screens;


import gui.BaseAnimation;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Parent;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressBar;
import javafx.scene.layout.Background;
import javafx.scene.layout.BackgroundFill;
import javafx.scene.layout.CornerRadii;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;


public class InitLoading extends BaseScreen {
	
	private ProgressBar bar = null;
	
	@Override
	protected Parent onShow() {
		Label lb = new Label();
		Label tip = new Label();
		
		ProgressBar bar = new ProgressBar(0);
	    bar.setPrefSize(300, 16);
	    bar.setProgress(progress);
		this.bar = bar;

		VBox bgbx = new VBox();
		bgbx.getChildren().addAll(lb, bar, tip);
		VBox.setMargin(lb, new Insets(8));
		VBox.setMargin(bar, new Insets(8));
		bgbx.setAlignment(Pos.CENTER);
		
		bgbx.getStylesheets().add(
		        getClass().getResource(
		            "loadingbar.css"
		        ).toExternalForm()
		    );

		lb.setTextFill(Color.ANTIQUEWHITE);
		lb.setScaleX(2);
		lb.setScaleY(lb.getScaleX());
		
        BackgroundFill bgf = new BackgroundFill(Color.BLACK, CornerRadii.EMPTY, Insets.EMPTY);
        Background bg = new Background(bgf);
        bgbx.setBackground(bg);
        
        lb.setText("Loading...");
        
        tip.setTextFill(Color.GRAY);
        tip.setAlignment(Pos.BOTTOM_CENTER);
        
        tip.setText("Every good game has a loading screen; naturally, this has to have it as well");
        
        return bgbx;
	}
	
	private int loadAmount = 0;
	private int loadedAmount = 0;
	private double progress = 0.0f;
	
	public void setProgress(double prog) {
		progress = prog;
	}
	
	private LoadingAnim lastAnim;

	public void incrLoad(int times) {
		setLoaded(getLoaded() + times);
		progress = Math.min(1, (double)getLoaded() / getLoadAmount());
		
		if (bar != null) {
			if (lastAnim != null) {
				lastAnim.stop();
			}
			lastAnim = new LoadingAnim(progress, 0.2, 0, 0.2);
		}
	}
	
	private class LoadingAnim extends BaseAnimation {
		private double np = 0;
		private double lp = 0;
	
        LoadingAnim(double newProgress, double dur, double delay, double ease) {
			super(dur, delay, ease);
			np = newProgress;
			lp = bar.getProgress();
		}

		@Override
        public void onTick(double fr) {
            bar.setProgress(BaseAnimation.Lerp(fr, lp, np));
        }

    }
	
	public void incrLoad() { incrLoad(1); }

	public int getLoadAmount() {
		return loadAmount;
	}
	
	public void setLoadAmount(int loadAmt) {
		this.loadAmount = loadAmt;
	}

	/**
	 * @return the loadedAmount
	 */
	public int getLoaded() {
		return loadedAmount;
	}

	/**
	 * @param loadedAmount the loadedAmount to set
	 */
	public void setLoaded(int loadedAmount) {
		this.loadedAmount = loadedAmount;
	}
	
}
