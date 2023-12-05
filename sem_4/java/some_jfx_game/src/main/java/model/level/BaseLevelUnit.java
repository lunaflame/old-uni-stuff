package level;

import javafx.geometry.Point2D;

public interface BaseLevelUnit {

	/**
	 * called after LevelParser constructs an instance of this unit
	 * @param pos the 2d position at which the corresponding character was encountered
	 */
	default void onParse(Point2D pos) { };
	
	/**
	 * called when LevelParser needs to add this unit to the level
	 * @param lv the level to add this unit to
	 */
	public abstract void addToLevel(BaseLevel lv);
}