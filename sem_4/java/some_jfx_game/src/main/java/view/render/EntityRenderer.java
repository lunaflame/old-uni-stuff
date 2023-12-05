package render;

import entities.BaseEntity;

// very questionable way of implementing renderers to be honest

// EntityRenderer is aware of what entity it's representing,
// but to get rendering data (ie where to draw, size, etc),
// it must be provided by a BaseRenderer

public abstract class EntityRenderer {
	public EntityRenderer(BaseEntity ent) {
		this.renderingEnt = ent;
	}
	
	// for override
	public abstract void render(BaseRenderer ren);
	
	// what the BaseRenderer will be calling; we automatically do
	// necessary ops here to take some headache off
	public final void doRender(BaseRenderer ren) {
		calcRenderBox(ren);
		render(ren);
	}

	protected static double[] renderBox = new double[3];
	
	protected void calcRenderBox(BaseRenderer ren) {
		BaseEntity ent = renderingEnt;
		assert ent != null;
		
		double x = ren.getPx(ent.getPos().getX()), y = ren.getPx(ent.getPos().getY());
		double sc = ren.scale(1);
		
		renderBox[0] = x; renderBox[1] = y; renderBox[2] = sc;
	}
	
	protected BaseEntity renderingEnt;
}
