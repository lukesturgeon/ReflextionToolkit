#ifndef _OF_BALL
#define _OF_BALL

#include "ofMain.h"
#include "ofxCsv.h"

// columns for saving data
#define TIME_COLUMN 0
#define DATA_COLUMN 1
#define PERSPECTIVE_COLUMN 2

class myReflex {
	
public:
	
	/*
	 * Constructor
	 */
	myReflex();
	
	/*
	 * Methods
	 */
	void clear();
	void setDataFromCsv(wng::ofxCsv* csv);
	int getCurrentFrame();
	int getTotalNumFrames();
	int getFrameRate();
	int getNumRows();
	
private:
	
	/*
	 * Variables
	 */
	int currentPerspective;
	int currentFrame;
	int totalFrames;
	int frameRate;
	vector<int> perspectiveTrack;
	vector<int> dataTrack;
	
};

#endif