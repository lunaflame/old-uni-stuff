package render;

import gui.BaseAnimation;
import javafx.geometry.VPos;
import javafx.scene.image.Image;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.FontPosture;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;
import javafx.scene.text.TextAlignment;
import screens.GameScreen;

public class HUDRenderer extends BaseRenderer {
	private static Image pacmanIcon;
	
	public HUDRenderer(GameScreen scr) {
		super(scr);
		var str = HUDRenderer.class.getResourceAsStream("/pacman.png");
		pacmanIcon = new Image(str, 14, 14, false, false);
	}
	
	Color boxBg = Color.rgb(30, 30, 30, 0.75);
	Color dim = Color.rgb(0, 0, 0, 0.8);
	
	private int scoreW = 300;
	private int scoreH = 48;
	
	private Font scoreFont = Font.font("Open Sans", FontWeight.BOLD, FontPosture.REGULAR, 32);
	private Font livesFont = Font.font("Open Sans", FontWeight.BOLD, FontPosture.REGULAR, 36);
	private Font gameOverFont = Font.font("Open Sans", FontWeight.BOLD, FontPosture.REGULAR, 64);

	private int livesW = 120;
	private int livesH = 48;

	private int rounding = 16;
	
	private void paintScore() {
		var ctx = getCtx();
		
		final String txStr = "Score: " + scr.getGame().getScore();
		final Text txObj = new Text(txStr);

		txObj.setFont(scoreFont);
		final double scoreWid = Math.max(txObj.getLayoutBounds().getWidth() + 16, scoreW);
		
		ctx.setFill(boxBg);
		ctx.fillRoundRect(getW() - scoreWid, -rounding, scoreWid + rounding, scoreH + rounding, rounding, rounding);
			
		ctx.setFill(Color.WHITE);
		ctx.setTextAlign(TextAlignment.CENTER);
		ctx.setTextBaseline(VPos.CENTER);
	
		ctx.setFont(scoreFont);
		ctx.fillText(txStr, getW() - scoreW / 2, scoreH / 2);
	}
	
	// can you spot where i started losing it?
	private void paintLyfes() {
		var ctx = getCtx();
		
		final String txStr = "x" + scr.getGame().getLives();
		final Text txObj = new Text(txStr);

		txObj.setFont(livesFont);
		final double tW = txObj.getLayoutBounds().getWidth();
		final double iconSize = livesH * 0.75;
		final double icPad = 8;
		final double totalW = tW + iconSize + icPad; // livesH = icon size
		final double boxW = Math.max(totalW + 16, livesW);
		
		ctx.setFill(boxBg);
		ctx.fillRoundRect(getW() - boxW, getH() - livesH, boxW + rounding, livesH + rounding, rounding, rounding);
			
		ctx.setFill(Color.WHITE);
		ctx.setTextAlign(TextAlignment.LEFT);
		ctx.setTextBaseline(VPos.CENTER);
	
		ctx.setFont(scoreFont);
		
		double x = getW() - boxW / 2 - totalW / 2;
		double y = getH() - livesH;
		
		ctx.drawImage(pacmanIcon, x, y + livesH / 2 - iconSize / 2, iconSize, iconSize);
		x += iconSize + icPad;
		
		ctx.fillText(txStr, x, y + livesH / 2);
	}
	
	private double dethWhen = 0;
	private double curFr = 0;
	private double dimLength = 0.8;
			
	private void paintGaemOvre() {
		var ctx = getCtx();
		var game = scr.getGame();
		
		if (game.isGameOver()) {
			dethWhen = dethWhen == 0 ? game.getGlobTime() : dethWhen;
			curFr = Math.min(dimLength, curFr + game.getFrametime());
			
		} else {
			dethWhen = 0;
			curFr = Math.max(0, curFr - game.getFrametime());
		}
		
		double fr = BaseAnimation.RemapClamp(curFr, 0, dimLength, 0, 1);
		fr = BaseAnimation.EaseFn(fr, 0.3);
		
		if (fr > 0) {
			ctx.setGlobalAlpha(fr);
			ctx.setFill(dim);
			ctx.fillRect(0, 0, getW(), getH());
			ctx.setGlobalAlpha(1);
		}

		if (game.isGameOver()) {
			if ((game.getGlobTime() - dethWhen) % 2 < 1) {
				final String txStr = "u r ded";
				final Text txObj = new Text(txStr);
				txObj.setFont(gameOverFont);
				final double tW = txObj.getLayoutBounds().getWidth();
				final double tH = txObj.getLayoutBounds().getHeight();
				
				ctx.setFont(gameOverFont);
				ctx.setTextAlign(TextAlignment.CENTER);
				ctx.setTextBaseline(VPos.CENTER);
				
				ctx.setFill(boxBg);
				
				ctx.fillRoundRect(
						getW() / 2 - tW / 2 - rounding, getH() / 2 - tH / 2,
						tW + rounding * 2, tH,
						rounding, rounding);
				
				ctx.setFill(Color.WHITE);
				ctx.fillText("u r ded", getW() / 2, getH() / 2);
			}
		}
	}
	
	@Override
	public void doRender() {
		paintScore();
		paintLyfes();
		paintGaemOvre();
	}

}
