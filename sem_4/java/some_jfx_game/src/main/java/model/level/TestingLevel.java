package level;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import cells.BaseCell;
import cells.Dot;
import cells.Wall;
import entities.Pacman;
import javafx.geometry.Point2D;


public class TestingLevel extends BaseLevel {

	public TestingLevel(GameState game) {
		LevelParser prs = LevelParser.get();
		String content = null;

		try {
			InputStream is = this.getClass().getResourceAsStream("/example_level.txt");
			if (is == null) { throw new IOException("it do be null doe"); }
			content = new String(is.readAllBytes(), StandardCharsets.UTF_8);
		} catch (IOException ex) {
				// ??
			System.out.println("Exception while opening input code file. " + ex.getMessage());
			return;
		}
		
		prs.parseInto(content, this);
		setGame(game);
		finish();
	}
}
