#pragma once

#include "ofMain.h"
#include "ofxCsv.h"
#include "myReflex.h"

#define PREVIEW_SIZE 480
#define SENSOR_PIN 0

using namespace wng;

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
	
		void setupArduino(const int & version);
		void analogPinChanged(const int & pinNum);
	
		// app layout and graphics
		ofImage background;
		ofRectangle previewWindow;
		ofRectangle playbackWindow;
	
		// recording variables
		ofVideoGrabber 			vidGrabber;
		ofPtr<ofQTKitGrabber>	vidRecorder;
		ofVideoPlayer recordedVideoPlayback;
		void videoSaved(ofVideoSavedEventArgs& e);
		vector<string> videoDevices;
		vector<string> audioDevices;
	
		// we load and parse the data in to these vectors
		vector<int> frameDataPlayback;
		vector<int> framePerspectivePlayback;
	
		// saving data
		ofxCsv dataRecorder;
		ofxCsv dataPlayback;
		int currentPerspective;
		unsigned long recordStartMillis;
	
		// all playback data is stored in an object
		myReflex reflex;
	
		
		// Arduino variables
		ofArduino	arduino;
		int sensorValue;
		int previewData[PREVIEW_SIZE];
	
		// utils
		string nf(int number, int digits);
	
};
