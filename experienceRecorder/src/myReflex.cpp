#include "myReflex.h"

//--------------------------------------------------------------
myReflex::myReflex() {
	currentFrame = 0;
	totalFrames = 0;
	frameRate = 17;
}

//--------------------------------------------------------------
void myReflex::setDataFromCsv(wng::ofxCsv* csv) {
	
	float millisPerFrame = frameRate / 1000.00;
	int duration = csv->getInt(csv->numRows-1, TIME_COLUMN);
	int frames = duration * millisPerFrame;
	
	cout << "millisPerFrame = " << millisPerFrame << ", duration = " << duration << ", frames =" << frames << endl;
	
//	float duration = ((float)) * 1000.00f;
	
	// get the total number of frames
//	cout << "millis = " <<  millis << ", seconds = " << seconds << ", frames = " << frames << endl;
	
	// convert the milliseconds to frame numbers	
//	cout << "[setDataAtMillis] " << time << "," << data << "," << perspective << endl;
}

//--------------------------------------------------------------
int myReflex::getCurrentFrame() {
	return currentFrame;
}

//--------------------------------------------------------------
int myReflex::getTotalNumFrames() {
	return totalFrames;
}

//--------------------------------------------------------------
int myReflex::getFrameRate() {
	return frameRate;
}


//--------------------------------------------------------------
void myReflex::clear() {
	currentFrame = 0;
	totalFrames = 0;
	perspectiveTrack.clear();
	dataTrack.clear();
}