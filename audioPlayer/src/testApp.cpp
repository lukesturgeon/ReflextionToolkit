#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	
	ofSetFrameRate(30.0f);
	ofSetCircleResolution(100.0f);
	ofSetLineWidth(3.0f);
	ofNoFill();
	
	// load the background
	background.loadImage("images/1280x720_background.png");
	
	// load the fonts
	ofTrueTypeFont::setGlobalDpi(72);
	futuraLight48.loadFont("fonts/FuturaStd-Light.ttf", 48);
	
	// the fft needs to be smoothed out, so we create an array of floats
	nBandsToGet = 64;
	fftVolume = 0.0f;
	fftSmoothed = new float[nBandsToGet];
	for (int i = 0; i < nBandsToGet; i++){
		fftSmoothed[i] = 0.0f;
	}
	
	// calculate the total length once (we don't need to keep re-calculating this)
	int totalTime = 60000 / 1000;
	totalSeconds = totalTime % 60;
	totalTime /= 60;
	totalMinutes = totalTime % 60;
	
	// load the audio track
	audio.loadSound("sound/final_reflection_1min.mp3");
	audio.setVolume(1.0f);
	audio.play();
}

//--------------------------------------------------------------
void testApp::update(){
	// update the OF sound engine
	ofSoundUpdate();
	
	float all = 0.0f;
	
	// request values from FFT
	float * val = ofSoundGetSpectrum(nBandsToGet);
	for (int i = 0; i < nBandsToGet; i++){
		
		// let the smoothed calue sink to zero:
		fftSmoothed[i] *= 0.96f;
		
		// take the max, either the smoothed or the incoming:
		if (fftSmoothed[i] < val[i]) fftSmoothed[i] = val[i];
		
		all += fftSmoothed[i];
	}
	
	// save one number that sums up the entire spectrum
	fftVolume = all * 0.5f;
}

//--------------------------------------------------------------
void testApp::draw(){
	
	background.draw(0.0f,0.0f);
	
	// draw the circles
	float offset = fftVolume * 30.0f;
	
	/*ofEllipse(ofGetWidth()*0.5f,
			  ofGetHeight() * 0.5f,
			  400.0f + offset,
			  400.0f + offset);*/
	
	// calculate the current time
	int currentTime = audio.getPositionMS() / 1000;
	int currentSeconds = currentTime % 60;
	currentTime /= 60;
	int currentMinutes = currentTime % 60;
	
	// draw the current time
	string s = nf(currentMinutes, 2) + ":" + nf(currentSeconds, 2) + "/" + nf(totalMinutes,2) + ":" + nf(totalSeconds,2);
	ofRectangle rect = futuraLight48.getStringBoundingBox(s, 0, 0);
	futuraLight48.drawString(s, (int)(ofGetWidth()*0.5f - (rect.width*0.5f) ), (ofGetHeight()*0.5f)+18.0f);
	
	cout << audio.getPositionMS() << endl;
}

//--------------------------------------------------------------
string testApp::nf(int number, int digits) {
	string format = "%0" + ofToString(digits) + "d";
    char buffer[100];
    sprintf(buffer, format.c_str(), number);
    return (string)buffer;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
