#pragma once

#include "ofMain.h"
#include "ofxCsv.h"

#define TRANSITION_READY 0
#define TRANSITION_OUT 1
#define TRANSITION_IN 2
#define TRANSITION_LOCK 3

#define DANCER_PERSPECTIVE 1
#define SPECTATOR_PERSPECTIVE 2
#define HIGH_VOLUME 1.0f
#define LOW_VOLUME 0.25f

#define BUTTON_PIN 2
#define SENSOR_PIN 0

#define SENSOR_PREVIEW_SIZE 600
#define USER_TIMEOUT 20000
#define EASING 0.4f
#define TOTAL_AUDIO_LENGTH 60000

#define TIME_COLUMN 0
#define SENSOR_COLUMN 1
#define PERSPECTIVE_COLUMN 2

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
	
		// app methods to get it working real nice
		string nf(int number, int digits);
		void goToFrame(int nextFrame);
	
		void pauseExperience();
		void resumeExperience();
		void resetExperience();
		void updatePerspectives();
	
		void setupArduino(const int & version);
		void digitalPinChanged(const int & pinNum);
		void analogPinChanged(const int & pinNum);
	
		// sensor stuff from arduino
		ofArduino	arduino;
		int sensorValue;
		bool sensorInUse;
		int sensorPreview[SENSOR_PREVIEW_SIZE];
	
		// image stuff for presentation
		vector <ofImage> frames;
		ofImage framePrompt;
		ofImage background;
		ofImage dancerWaveform;
		ofImage spectatorWaveform;
		int framePromptAlpha;
		int nextFrameIndex;
		int currentFrameIndex;
		int transitionState;
		float currentFramePosX;
		float underlineStart;
		float underlineEnd;
		float memoryPlayheadX;
	
		ofRectangle bounds;
		ofRectangle dancer;
		ofRectangle spectator;
		ofPolyline sensorDataLine;
		vector<ofRectangle> dancerDataLine;
		vector<ofRectangle> spectatorDataLine;
		int minSensorValue;
		int maxSensorValue;
		
		// set the underline positions
		float underlineTargets[10][2] =
		{
			{ 313,	427 },	// probe
			{ 482,	608 },	// performance
			{ 482,	608 },	// comfortable
			{ 671,	769 },	// expectations
			{ 671,	769 },	// performance
			{ 835,	957 },	// interpretation
			{ 835,	957 },	// response
			{ 1020,	1120 }, // memory
			{ 1020,	1120 }, // reflection
			{ 1020,	1120 }	// thanks
		};
	
		// set the helper text
		string helperText[10] =
		{
			"Press SPACE to move forward",
			"Press SPACE to move forward",
			"Press SPACE to move forward",
			"Press SPACE to begin",
			"Press LEFT and RIGHT to switch perspective",
			"Press SPACE to move forward",
			"Press SPACE to move forward",
			"Press SPACE to move forward",
			"Press SPACE to move forward",
			"A project by Luke Sturgeon"
		};
	
		// sound stuff for experience
		ofSoundPlayer  dancerStory;
		ofSoundPlayer  spectatorStory;
		int currentPerspective;
		float spectatorDiameter;
		float spectatorPosX;
		float dancerDiameter;
		float dancerPosX;
		float spectatorDiameterTarget;
		float spectatorPosXTarget;
		float dancerDiameterTarget;
		float dancerPosXTarget;
		int spectatorDurationSeconds;
		int spectatorDurationMinutes;
	
		float * fftSmoothed;
		int nBandsToGet;
		float fftVolume;
	
		// app stuff
		unsigned long lastUserInteraction;
		unsigned long startPerspectiveMillis;
		bool paused;
		bool debugging;
	
		ofTrueTypeFont futuraLight48;
		ofTrueTypeFont futuraBook18;
	
		wng::ofxCsv csv;
		
};
