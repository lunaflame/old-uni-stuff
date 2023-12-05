package render;

import java.util.ArrayList;
import java.util.HashMap;

import entities.BaseGhost;
import entities.BaseMovableEntity.Direction;
import javafx.animation.AnimationTimer;
import javafx.scene.effect.Blend;
import javafx.scene.effect.BlendMode;
import javafx.scene.effect.ColorAdjust;
import javafx.scene.effect.Light;
import javafx.scene.effect.Lighting;
import javafx.scene.image.Image;
import javafx.scene.paint.Color;

public class GhostRenderer extends EntityRenderer {
	public GhostRenderer(BaseGhost ghost) {
		super(ghost);
		load();
	}
	
	@Override
	public void render(BaseRenderer ren) {
		var ghost = (BaseGhost)renderingEnt;
		
		var ctx = ren.getCtx();
		ctx.setImageSmoothing(false);
		calcRenderBox(ren);
		
		int useFrame = (int) Math.floor( (curtime % (double)(frames / fps)) * (double)fps );

		var imgSet = dirImg.get(ghost.getDirection());
		if (imgSet == null) {
			imgSet = dirImg.get(Direction.UP);
		}
		
		if (imgSet != null) {
			Image img = imgSet.get(useFrame);
			ctx.setFill(getColor());
			
			// cope
			Lighting lighting = new Lighting(new Light.Distant(45, 90, getColor()));
			ColorAdjust bright = new ColorAdjust(0, 1, 1, 1);
			lighting.setContentInput(bright);
			lighting.setSurfaceScale(0.0);

			ctx.setEffect(lighting);
			
			if (img != null) {
				ctx.drawImage(img, renderBox[0], renderBox[1], renderBox[2], renderBox[2]);
			} else {
				ctx.fillOval(renderBox[0], renderBox[1], renderBox[2], renderBox[2]);
			}
			
			ctx.setEffect(null);
		}
		
		double x = ren.getPx(ghost.getTargetCell().getX()), y = ren.getPx(ghost.getTargetCell().getY());
		double one = ren.scale(1);
		double sc = ren.scale(0.4);

		ctx.setStroke(getColor());
		ctx.strokeOval(x + one / 2 - sc / 2, y + one / 2 - sc / 2, sc, sc);
	}
	
	public Color getColor() {
		return clr;
	}

	public void setColor(Color clr) {
		this.clr = clr;
	}
	
	private static HashMap<Direction, ArrayList<Image>> dirImg = new HashMap<> ();
	private static boolean loaded = false;
	private static double curtime = 0;
	
	static private double fps = 3;
	static private int frames = 2;

	private static AnimationTimer timer = new AnimationTimer() {
	    @Override
	    public void handle(long now) {
	    	curtime = (double) (now / 1e9);
	    }
	};
	
	private static void load() {
		if (loaded) return;
		loaded = true;
		timer.start();
		
		for (Direction dir : Direction.values()) {
			if (dir == Direction.NONE) { continue; }
			
			char pfx = dir.name().toLowerCase().charAt(0);
			
			var arr = new ArrayList<Image>();
			
			for (int i = 1; i <= 2; i++) {
				String path = String.format("/ghosts/%c%d.png", pfx, i);
				var th = GhostRenderer.class.getResourceAsStream(path);
				
				if (th == null) {
					System.out.println("Missing ghost sprite @ " + path);
					continue;
				}
				 
				arr.add(new Image(th, 14, 14, false, false));
			}
			
			dirImg.put(dir, arr);
		}
	}
	
	private Color clr = Color.WHITE;
}
