package gui;

import java.util.function.DoubleFunction;
import javafx.animation.AnimationTimer;

// i don't like jfx's animation api
public class BaseAnimation extends AnimationTimer {
	private void setParams(double dur, double del) {
		duration = dur;
		delay = del;
	}
	
	private double defaultEaseNum = 1;
	
	public static double EaseFn(double x, double easeNum) {
		if (easeNum < 0) {
			return Math.pow( x, Math.pow(1 - (x - 0.5), -easeNum) );
		} else if (easeNum > 0 && easeNum < 1) {
			return 1 - Math.pow(1 - x, 1 / easeNum);
		} else {
			// ease in
			return Math.pow(x, easeNum);
		}
	}
	
	private DoubleFunction<Double> customEase = null;
	
	protected BaseAnimation(double dur, double delay, DoubleFunction<Double> easeFn) { // i hate jaba
		setParams(dur, delay);
		customEase = easeFn;
		start();
	}
	
	protected BaseAnimation(double dur, double delay, double ease) {
		setParams(dur, delay);
		defaultEaseNum = ease;
		start();
	}
	
	private long startTime = 0;
	
	@Override
	public void handle(long now) {
		if (duration == 0) {
			throw new IllegalArgumentException("Animation Duration is 0");
		}
		if (startTime == 0) {
			startTime = now;
		}
		
		long passed = now - startTime - (long)(delay * 1e9);
		if (passed < 0) { return; } // delay still active
		
		double fr = (double)passed / (duration * 1e9);
		
		if (customEase != null) {
			fr = customEase.apply(fr);
		} else {
			fr = EaseFn(fr, defaultEaseNum);
		}
		
		fr = Math.min(1, fr);
		onTick(fr);
		
		if (fr >= 1) {
			onStop();
			stop();
		}
	}
	
	static public double Lerp(double fr, double from, double to) {
		if (fr > 1) { return to; }
		if (fr < 0) { return fr; }
		
		return from + (to - from) * fr;
	}
	
	static public double RemapClamp(double val, double lo, double hi, double lo2, double hi2) {
		return lo2 + Math.max(0, Math.min(1, (val - lo) / (hi - lo))) * (hi2 - lo2);
	}
	/**
	 * @param frac animation progress (from 0 to 1)
	 */
	protected void onTick(double frac) { };
	protected void onStop() { };
	
	private double duration = 0;
	private double delay = 0;
}
