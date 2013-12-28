#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetFrameRate(30);
    ofSetVerticalSync(true);
	
	// load images for background
	background.loadImage("images/background.png");
	
	// set variables for recording
	currentPerspective = 0;
	previewWindow = ofRectangle(98, 100, 480, 360);
    playbackWindow = ofRectangle(702, 100, 480, 360);
	
	// Create a new recorder object
	vidRecorder = ofPtr<ofQTKitGrabber>( new ofQTKitGrabber() );
	
	// Set our video grabber to use this source
	vidGrabber.setGrabber(vidRecorder);
	
	// Make lists of our audio and video devices.
    videoDevices = vidRecorder->listVideoDevices();
    audioDevices = vidRecorder->listAudioDevices();
	
	// Add audio to the recording stream.
    vidRecorder->setAudioDeviceID(1);
    vidRecorder->setUseAudio(true);
	
	// Register for events so we'll know when videos finish saving.
	ofAddListener(vidRecorder->videoSavedEvent, this, &testApp::videoSaved);
	
	// Initialize the grabber.
    vidGrabber.initGrabber(640, 480);
	vidRecorder->initRecording();
	
	// connect to the arduino using ofArduino
	arduino.connect("/dev/tty.usbmodem621", 57600);
	
	// listen for EInitialized event
	// this indicates that the arduino is ready to receive commands
	// and it is safe to call setupArduino()
	ofAddListener(arduino.EInitialized, this, &testApp::setupArduino);
	
}

//--------------------------------------------------------------
void testApp::update(){
	
	// update the QT video grabber
	vidGrabber.update();
	
	if(recordedVideoPlayback.isLoaded()){
        recordedVideoPlayback.update();
    }
	
	// update the arduino, get any data or messages
	arduino.update();
	
	// nudge down all the preview data
	for (int i = 0; i < PREVIEW_SIZE; i++) {
		previewData[i] = previewData[i+1];
	}
	
	// append the new value
	previewData[PREVIEW_SIZE-1] = sensorValue;
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(255);
	background.draw(0,0);
	
	//-------------------------------------
	// draw the background boxes
    ofPushStyle();
    ofSetColor(0);
    ofFill();
    ofRect(previewWindow);
    ofRect(playbackWindow);
    ofPopStyle();
	
	//-------------------------------------
	// draw the preview if available
	if(vidRecorder->hasPreview()){
        ofPushStyle();
        ofFill();
        ofSetColor(255);
        // fit it into the preview window, but use the correct aspect ratio
        ofRectangle videoGrabberRect(0,0,vidGrabber.getWidth(),vidGrabber.getHeight());
        videoGrabberRect.scaleTo(previewWindow);
        vidGrabber.draw(videoGrabberRect);
        ofPopStyle();
    } else{
		ofPushStyle();
		// x out to show there is no video preview
        ofSetColor(255);
		ofSetLineWidth(3);
		ofLine(20, 20, 640+20, 480+20);
		ofLine(20+640, 20, 20, 480+20);
		ofPopStyle();
	}
	
	//-------------------------------------
	// draw the playback video
    if(recordedVideoPlayback.isLoaded()){
        ofPushStyle();
        ofFill();
        ofSetColor(255);
        // fit it into the preview window, but use the correct aspect ratio
        ofRectangle recordedRect(ofRectangle(0,0,recordedVideoPlayback.getWidth(),recordedVideoPlayback.getHeight()));
        recordedRect.scaleTo(playbackWindow);
        recordedVideoPlayback.draw(recordedRect);
        ofPopStyle();
    }
	
	//-------------------------------------
	// draw the recording box
	ofPushStyle();
    ofNoFill();
    ofSetLineWidth(3);
    if(vidRecorder->isRecording()){
        //make a nice flashy red record color
        int flashRed = powf(1 - (sin(ofGetElapsedTimef()*10)*.5+.5),2)*255;
		ofSetColor(255, 255-flashRed, 255-flashRed);
    }
    else{
    	ofSetColor(255,80);
    }
	ofPushMatrix();
	ofTranslate(-0.5f, -0.5f);
    ofRect(previewWindow);
	ofPopMatrix();
    ofPopStyle();
	
	//-------------------------------------
	// draw the live graph
	ofPushMatrix();
	ofTranslate(98.5f, previewWindow.y+previewWindow.height+0.5f);
	ofSetColor(255);
	ofNoFill();
	ofSetLineWidth(1);
	ofRect(0, 0, PREVIEW_SIZE, 100);
	ofTranslate(0, -0.5f);
	ofBeginShape();
	
	for (int i = 0; i < PREVIEW_SIZE; i++) {
		ofVertex(i, ofMap(previewData[i], 400, 650, 0, 100, true));
	}
	ofEndShape();
	ofTranslate(0, 0.5f);
	ofDrawBitmapString("Live data", 2, 12);
	ofPopMatrix();
	
	//-------------------------------------
	// draw the recorded video data
	ofPushMatrix();
	ofTranslate(playbackWindow.x+0.5f, playbackWindow.y+playbackWindow.height+0.5f);
	ofRect(0, 0, PREVIEW_SIZE, 100);
	ofTranslate(-0.5f, -0.5f);
	ofBeginShape();
	for (int i = 0; i < dataPlayback.numRows; i++) {
		float x = ofMap(i, 0, dataPlayback.numRows, 0, PREVIEW_SIZE);
		ofCurveVertex(x, ofMap(dataPlayback.getInt(i, DATA_COLUMN), 400, 650, 0, 100, true));
	}
	ofEndShape();
	
	
	if (recordedVideoPlayback.isPlaying() == true) {
		// playhead (convert currentFrame to x position within the rectangle)
		int playhead = ofMap(recordedVideoPlayback.getCurrentFrame(), 0, recordedVideoPlayback.getTotalNumFrames(), 0, PREVIEW_SIZE);
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofLine(playhead, 0, playhead, 100);
		ofPopStyle();
	}
	ofDrawBitmapString("Saved data", 2, 12);
	ofPopMatrix();
	
	
	//-----------------
	ofPushMatrix();
	ofTranslate(0, 30);
	//draw instructions
    ofPushStyle();
    ofDrawBitmapString("TOGGLE CONTROLS", 98, 590);
	ofDrawBitmapString("' ' recording", 98, 610);
    ofDrawBitmapString("'v' video device", 98, 630);
    ofDrawBitmapString("'a' audio device", 98, 650);
	
	//draw video device selection
    ofDrawBitmapString("VIDEO DEVICE", 256, 590);
    for(int i = 0; i < videoDevices.size(); i++){
        if(i == vidRecorder->getVideoDeviceID()){
			ofSetColor(255, 112, 112);
        }
        else{
            ofSetColor(255);
        }
        ofDrawBitmapString(videoDevices[i], 256, 610+i*20);
    }
	ofPopStyle();
	
	//draw audio device;
	ofPushStyle();
    ofDrawBitmapString("AUDIO DEVICE", 415, 590);
    for(int i = 0; i < audioDevices.size(); i++){
        if(i == vidRecorder->getAudioDeviceID()){
			ofSetColor(255, 112, 112);
        }
        else{
            ofSetColor(255);
        }
        ofDrawBitmapString(audioDevices[i], 415, 610+i*20);
    }
	ofPopStyle();
	
	
	// draw playback data
	ofDrawBitmapString("VIDEO PLAYBACK", 702, 590);
	ofDrawBitmapString("Frame: " + ofToString(recordedVideoPlayback.getCurrentFrame()) + "/" + ofToString(recordedVideoPlayback.getTotalNumFrames()), 702, 610);
	
	int frameRate = 17;
	string hh = "00";
	string mm = "00";
	string ss = nf(recordedVideoPlayback.getCurrentFrame() / frameRate, 2);
	string ff = nf(recordedVideoPlayback.getCurrentFrame() % frameRate, 2);
	ofDrawBitmapString(hh+":"+mm+":"+ss+":"+ff, 702, 630);
	
	ofDrawBitmapString("REFLEXTION", 860, 590);
	ofDrawBitmapString("Framerate: " + ofToString(reflex.getFrameRate()) + "fps", 860, 610);
	ofDrawBitmapString("Frame: " + ofToString(reflex.getCurrentFrame()) + "/" + ofToString(reflex.getTotalNumFrames()), 860, 630);
	ofDrawBitmapString("Perspective: ", 860, 650);
	
	ofPopMatrix();
	//------------------
}

//--------------------------------------------------------------
void testApp::videoSaved(ofVideoSavedEventArgs& e) {
	
	// the ofQTKitGrabber sends a message with the file name and any errors when the video is done recording
	if ( e.error.empty() ){
	    recordedVideoPlayback.loadMovie(e.videoPath);
	    recordedVideoPlayback.play();
		dataRecorder.clear();
		
		// load and convert the data
		dataPlayback.loadFile(ofToDataPath("MyDataFile.csv"));
		reflex.setDataFromCsv(&dataPlayback);
		cout << "just loaded " << dataPlayback.numRows << " rows of data" << endl;
		
		
	}
	else {
		ofLogError("videoSavedEvent") << "Video save error: " << e.error;
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	if(key == ' '){
		
        //if it is recording, stop
        if(vidRecorder->isRecording()){
			// stop the video
            vidRecorder->stopRecording();
			
			// stop the data and save all the rows in the Table
			dataRecorder.saveFile( ofToDataPath( "MyDataFile.csv" ), "," , "GSR data from Arduino");
        }
        else {
            // otherwise start a new recording.
            // before starting, make sure that the video file
            // is already in use by us (i.e. being played), or else
            // we won't be able to record over it.
            if(recordedVideoPlayback.isLoaded()){
                recordedVideoPlayback.close();
            }
			recordStartMillis = ofGetElapsedTimeMillis();
			dataPlayback.clear();
	        vidRecorder->startRecording("MyMovieFile.mov");
        }
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
	if(key == 'v'){
		vidRecorder->setVideoDeviceID( (vidRecorder->getVideoDeviceID()+1) % videoDevices.size() );
    }
	if(key == 'a'){
        vidRecorder->setAudioDeviceID( (vidRecorder->getAudioDeviceID()+1) % audioDevices.size() );
    }
}

//--------------------------------------------------------------
string testApp::nf(int number, int digits) {
	string format = "%0" + ofToString(digits) + "d";
    char buffer[100];
    sprintf(buffer, format.c_str(), number);
    return (string)buffer;
}

//--------------------------------------------------------------
void testApp::setupArduino(const int & version) {
	
	// remove listener because we don't need it anymore
	ofRemoveListener(arduino.EInitialized, this, &testApp::setupArduino);
    
    // print firmware name and version to the console
    ofLogNotice() << arduino.getFirmwareName();
    ofLogNotice() << "firmata v" << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();
	
    // set pin A0 to analog input
    arduino.sendAnalogPinReporting(SENSOR_PIN, ARD_ANALOG);
	
    // Listen for changes on the digital and analog pins
    ofAddListener(arduino.EAnalogPinChanged, this, &testApp::analogPinChanged);
}

//--------------------------------------------------------------
void testApp::analogPinChanged(const int & pinNum) {
	//    ofLogNotice() << "analog pin: " << pinNum << " = " << arduino.getAnalog(pinNum) << endl;
	
	if (pinNum == SENSOR_PIN) {
		// set the sensorValue value
		sensorValue = arduino.getAnalog(pinNum);
		
		// if we are recording, add data to csv
		if (vidRecorder->isRecording() == true) {
			int next_row = dataRecorder.numRows;
			// save the millise in the time column (column 0)

			cout << "[" << (ofGetElapsedTimeMillis() - recordStartMillis) << "] " << TIME_COLUMN << "," << sensorValue << endl;

			dataRecorder.setInt(next_row, TIME_COLUMN, ofGetElapsedTimeMillis() - recordStartMillis);
			dataRecorder.setInt(next_row, DATA_COLUMN, sensorValue);
			dataRecorder.setInt(next_row, PERSPECTIVE_COLUMN, currentPerspective);
		}
	}
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
