package bfk;

import java.util.HashMap;

// class for interfacing with ops
public class BFOpInterface {
	// Custom data can be used by operations to share data between ops at different points in time
	public Object GetCustomData(String key) {
		return customData.get(key);
	}
		
	public void SetCustomData(String key, Object value) {
		customData.put(key, value);
	}
	
	public void SetCustomDataSet(HashMap<String, Object> newData) {
		customData = newData;
	}
	
	public HashMap<String, Object> GetCustomDataSet() {
		return customData;
	}
	
	public void ClearCustomData() {
		customData.clear();
	}

	private HashMap<String, Object> customData = new HashMap<>();
}
