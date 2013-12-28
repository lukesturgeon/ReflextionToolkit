#pragma once

#include "ofMain.h"
#include "ofxCsv.h"

#define TIME_COLUMN 0
#define SENSOR_COLUMN 1
#define PERSPECTIVE_COLUMN 2

#define DANCER_PERSPECTIVE 1
#define SPECTATOR_PERSPECTIVE 2

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
		string nf(int number, int digits);
	
		ofImage background;
	
		string filename;
		wng::ofxCsv csv;
		int minSensorValue;
		int maxSensorValue;
		ofRectangle bounds;
		ofRectangle dancer;
		ofRectangle spectator;
		ofPolyline sensorDataLine;
		vector<ofRectangle> dancerDataLine;
		vector<ofRectangle> spectatorDataLine;
		bool showInterval;
		float showIntervalX;
	
		// preview data for the graph
		float sPosition;
		int sRow;
		string sTime;
		int sSensor;
		string sPerspective;
	
		ofTrueTypeFont futuraBook14;
		
};
