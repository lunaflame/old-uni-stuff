package render;

import entities.BaseEntity;
import level.BaseLevel;
import screens.GameScreen;

public class BaseEntityRenderer extends BaseRenderer {

	public BaseEntityRenderer(GameScreen scr) {
		super(scr);
		// TODO Auto-generated constructor stub
	}
	
	public void doRender() {
		BaseLevel lv = scr.getLevel();
		
		double w = getW(), h = getH();
		
		// a single unit's rendered scale
		double scale = Math.min(w / lv.getW(), h / lv.getH());
		this.scale = Math.floor(scale);

		var ents = lv.getEnts();
		
		for (Object oent : ents) {
			BaseEntity ent = (BaseEntity)oent;
			
			var ren = ent.getRenderer();
			if (ren == null) {
				System.out.println("Entity " + ent + " is missing a renderer!");
				continue;
			}
			
			ren.render(this);
		}
	}
}
