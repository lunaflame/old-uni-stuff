package screens;

import javafx.scene.control.Button;
import javafx.scene.layout.Pane;

public abstract class BaseMenuScreen extends BaseScreen {
	
	private Pane panePar = null;
	
	public BaseMenuScreen() {
		// TODO Auto-generated constructor stubs
	}
	
	/*
	private class Choice {
		int row = 0;
		int col = 0;
		Button btn;
		
		Choice(Button btn, int row, int col) {
			this.row = row; this.col = col; this.btn = btn;
		}
	}*/
	
	public void addButton(Button btn) {
		Pane par = getPaneParent();
		if (par == null) { throw new NullPointerException(); }
		
		//Choice c = new Choice(btn, row, col);
		par.getChildren().add(btn);
		//choices.add(c);
	}

	/**
	 * @return the panePar
	 */
	public Pane getPaneParent() {
		return panePar;
	}

	/**
	 * @param panePar the panePar to set
	 */
	public void setPaneParent(Pane panePar) {
		this.panePar = panePar;
	}
	
	/*public void setSelectedBtn(Button btn) {
		for (Choice c : choices) {
			if (c.btn == btn) {
				doSelect(c);
				break;
			}
		}
	}
	
	public Button getSelectedBtn() {
		return curChoice.btn;
	}
	
	public void onKeyPressed(KeyEvent key) {
		KeyCode code = key.getCode();
		
		// not very pretty but a hashtable + wrapper for a function is overkill
		switch (code) {
		case DOWN:
			selectVert(-1);
			break;
		case UP:
			selectVert(1);
			break;
		case RIGHT:
			selectHor(1);
			break;
		case LEFT:
			selectHor(-1);
			break;
		default:
			return;
		}
	}
	
	private void doSelect(Choice ch) {
		curChoice = ch;
	}
	private void selectVert(int dir) {
		int cR = 0;
		if (curChoice != null) {
			cR = curChoice.row;
		}
		

		// going vertically; compare rows
		for (Choice c : choices) {
			if (c.row == cR + 1 * dir) {
				curChoice = c;
				break;
			}
		}
		
		// if we're here then we didn't find anything;
		// wrap around to the top (or bottom if going up)
		
		int wrapRow = Integer.MAX_VALUE * -dir; // if going down, we need the highest (smallest) row
		Choice wrapTo = curChoice;
		
		for (Choice c : choices) {
			if ( (dir == -1 && c.row < wrapRow) || (dir == 1 && c.row > wrapRow) ) {
				wrapRow = c.row;
				wrapTo = c;
			}
		}
		
		if (wrapTo != null) {
			doSelect(wrapTo);
		} else {
			System.out.println("this should never happen!...");
		}
	}
	
	private void selectHor(int dir) {
		
	}

	private ArrayList<Choice> choices = new ArrayList<> ();
	private Choice curChoice = null;*/
}
