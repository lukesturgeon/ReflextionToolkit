#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetFrameRate(30);
	ofBackground(255);
	ofSetCircleResolution(64);
	ofDisableSmoothing();
	
	// load the fonts
	ofTrueTypeFont::setGlobalDpi(72);
	futuraLight48.loadFont("fonts/FuturaStd-Light.ttf", 48);
	futuraBook18.loadFont("fonts/FuturaStd-Book.ttf", 18);
	futuraBook18.setSpaceSize(0.5f);
	
	// calculate the areas for the user memory graph
	bounds = ofRectangle(120, 220.0f, ofGetWidth()-240, 100);
	dancer = ofRectangle(120, 320.0f, bounds.width, 100);
	spectator = ofRectangle(120, 420.0f, bounds.width, 100);
	
	// load the audio tracks
	dancerStory.loadSound("audio/dancer_story.mp3");
	dancerStory.setVolume(HIGH_VOLUME);
	dancerStory.setPan(0.0f); // set to middle
	
	spectatorStory.loadSound("audio/spectator_story.mp3");
	spectatorStory.setVolume(LOW_VOLUME);
	spectatorStory.setPan(1.0f); // set to right
	
	// calculate the total length once (we don't need to keep re-calculating this)
	int totalTime = TOTAL_AUDIO_LENGTH / 1000;
	spectatorDurationSeconds = totalTime % 60;
	totalTime /= 60;
	spectatorDurationMinutes = totalTime % 60;
	
	ofLogNotice() << "Total sound time" << totalTime << endl;
	
	// we should start at the first image
	resetExperience();
	currentFramePosX = 0;
	framePromptAlpha = 255;
	
	noiseStep = 0;
	noiseAmount = 0.15;
	debugging = false;
	nBandsToGet = 64;
	dancerPosX = ofGetWidth();
	spectatorPosX = ofGetWidth();
	
	// the fft needs to be smoothed out, so we create an array of floats
	fftSmoothed = new float[nBandsToGet];
	for (int i = 0; i < nBandsToGet; i++){
		fftSmoothed[i] = 0.0f;
	}
	
	// call the updatePerspectives() function to calculate graphics
	updatePerspectives();
	
	// load the static images
	background.loadImage("images/interface_flow.000.png");
	framePrompt.loadImage("images/interface_flow.001.png");
	dancerWaveform.loadImage("images/dancer-waveform.png");
	spectatorWaveform.loadImage("images/spectator-waveform.png");
	
	// load all interface pages
	for (int i = 2; i <= 11; i++) {
		// add the image to the vector
		string filePath = "images/interface_flow."+nf(i, 3)+".png";
		frames.push_back( ofImage() );
		frames.back().loadImage( filePath );
	}
	
	// default Arduino values
	sensorValue = 0;
	sensorInUse = false;
	
	// connect to the arduino using ofArduino
//	arduino.connect("/dev/tty.usbmodem621", 57600);
	
	// listen for EInitialized event
	// this indicates that the arduino is ready to receive commands
	// and it is safe to call setupArduino()
//	ofAddListener(arduino.EInitialized, this, &testApp::setupArduino);
}

//--------------------------------------------------------------
void testApp::update() {
	
	// update the arduino, get any data or messages
//	arduino.update();
	
	// nudge down all the preview data
	for (int i = 0; i < SENSOR_PREVIEW_SIZE; i++) {
		sensorPreview[i] = sensorPreview[i+1];
	}
	
	// append the new value
	sensorValue = ofMap( ofSignedNoise(noiseStep), -1, 1, 350, 650 );
	noiseStep += 0.001;
	
	sensorPreview[SENSOR_PREVIEW_SIZE-1] = sensorValue;
	
	
	// update the OF sound engine
	ofSoundUpdate();
	
	// SLIDE OUT
	if (transitionState == TRANSITION_OUT) {
		
		// EASE the position
		float dx = (0.0f-ofGetWidth()) - currentFramePosX;
		currentFramePosX += dx * EASING;
		
		// EASE the underline
		underlineEnd += (underlineTargets[nextFrameIndex][1] - underlineEnd) * EASING;
		
		// finshed fade out, so fade in
		if (abs(dx) <= 1.0f) {
			currentFrameIndex = nextFrameIndex;
			currentFramePosX = ofGetWidth();
			transitionState = TRANSITION_IN;
		}
	}
	// SLIDE IN
	else if (transitionState == TRANSITION_IN) {
		
		// EASE the position
		float dx = 0.0f - currentFramePosX;
		currentFramePosX += dx * EASING;
		
		// EASE the underline
		underlineStart += (underlineTargets[currentFrameIndex][0] - underlineStart) * EASING;
		
		// finished fade in, so stop
		if (abs(dx) <= 1.0f) {
			currentFramePosX = 0;
			
			// Decide what to show now the frame is revealed
			if ( currentFrameIndex == 4 ) {
				// interactive story
				transitionState = TRANSITION_LOCK;
				startPerspectiveMillis = ofGetElapsedTimeMillis();								
				dancerStory.play();
				spectatorStory.play();
				
				dancerStory.setPosition(0.80f);
				spectatorStory.setPosition(0.80f);

				
				ofLogNotice() << "start playing experience" << endl;
			}
			else if (currentFrameIndex == 7) {
				ofLogNotice() << "show the data";
				// interactive story
				transitionState = TRANSITION_LOCK;
				memoryPlayheadX = bounds.x;
				minSensorValue = 1023;
				maxSensorValue = 0;
				
				int tmpValue;
				
				// measure the min/max values
				for (int i = 0; i < csv.numRows; i++) {
					tmpValue = csv.getInt(i, SENSOR_COLUMN);
					
					if ( tmpValue > maxSensorValue) {
						maxSensorValue = tmpValue;
					}
					else if ( tmpValue < minSensorValue ) {
						minSensorValue = tmpValue;
					}
				}
				
				ofLog() << "minSensorValue " << minSensorValue;
				ofLog() << "maxSensorValue " << maxSensorValue;
				
				// loop through everythhing and calculate the graph ONCE
				int mappedIndex = 0;
				float mappedPosX = 0.0f;
				int dataLength = MIN(bounds.width, csv.numRows);
				int prevPerspective = 0;
				int dataPerspective = 0;
				float startX = 0.0f;
				
				for (int i = 0; i < dataLength; i++) {
					mappedPosX = ofMap(i, 0.0f, dataLength, bounds.x, bounds.x+bounds.width);
					mappedIndex = ofMap(i, 0.0f, dataLength, 0.0f, csv.numRows);
					
					// save the mapped graph lines to draw in the loop
					sensorDataLine.addVertex(mappedPosX,
											 ofMap(csv.getInt(mappedIndex, SENSOR_COLUMN), minSensorValue, maxSensorValue, bounds.y+bounds.height, bounds.y));
					
					// get the current perspective and start drawing boxes
					dataPerspective = csv.getInt(mappedIndex, PERSPECTIVE_COLUMN);
					
					if (dataPerspective != prevPerspective) {
						if (prevPerspective == DANCER_PERSPECTIVE) {
							dancerDataLine.push_back( ofRectangle(startX, dancer.y, mappedPosX-startX, dancer.height) );
						}
						else if (prevPerspective == SPECTATOR_PERSPECTIVE) {
							spectatorDataLine.push_back( ofRectangle(startX, spectator.y, mappedPosX-startX, spectator.height) );
						}
						
						// we've switched so remember the start
						startX = mappedPosX;
					}
					
					// remember the current for the next loops
					prevPerspective = dataPerspective;
				}
				
				// and once the loop's completed, we should close the final rect
				if (prevPerspective == DANCER_PERSPECTIVE) {
					dancerDataLine.push_back( ofRectangle(startX, dancer.y, (mappedPosX-startX)+1, dancer.height) );
				}
				else if (prevPerspective == SPECTATOR_PERSPECTIVE) {
					spectatorDataLine.push_back( ofRectangle(startX, spectator.y, (mappedPosX-startX+1), spectator.height) );
				}
			}
			else {
				transitionState = TRANSITION_READY;
			}
		}
	}
	
	// PROMPT SHOW (no one is touching)
	if ( sensorInUse == false && framePromptAlpha < 255) {
		framePromptAlpha += 40;
		
		// if finished fade, stop
		if (framePromptAlpha >= 255) {
			framePromptAlpha = 255;
			
			// fire the event
			pauseExperience();	
		}
	}
	// PROMPT HIDE (someone is touching)
	else if ( sensorInUse == true && framePromptAlpha > 0) {
		framePromptAlpha -= 40;
		
		// if finished fade
		if (framePromptAlpha <= 0) {
			framePromptAlpha = 0;
			
			// fire the resume experence
			resumeExperience();
		}
	}
	
	// check if someone has STOPPED using it after 10 secs
	if (paused == true && ( ofGetElapsedTimeMillis() - lastUserInteraction > USER_TIMEOUT)) {
		resetExperience();
	}
	
	// check and animate the experience
	if (paused == false && transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
		
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
		
		// animate the experience graphics
		spectatorPosX += (spectatorPosXTarget - spectatorPosX) * 0.5f;
		spectatorDiameter += (spectatorDiameterTarget - spectatorDiameter) * 0.5f;
		dancerPosX += (dancerPosXTarget - dancerPosX) * 0.5f;
		dancerDiameter += (dancerDiameterTarget - dancerDiameter) * 0.5f;
		
		// we are recording, so save the user's data
		int nextRow = csv.numRows;
		csv.setInt(nextRow, TIME_COLUMN, ofGetElapsedTimeMillis()-startPerspectiveMillis);
		csv.setInt(nextRow, SENSOR_COLUMN, sensorValue);
		csv.setInt(nextRow, PERSPECTIVE_COLUMN, currentPerspective);
//		ofLogNotice() << "[" << (ofGetElapsedTimeMillis()-startPerspectiveMillis) << "] " << sensorValue << " / " << currentPerspective;
		
		if (spectatorStory.getIsPlaying() == false) {
			ofLogNotice() << "we saved " << csv.numRows << "rows of data";
			
			// we've reached the end, so unlock and move onwards
			transitionState = TRANSITION_READY;
			csv.saveFile( ofToDataPath( "MyDataFile.csv" ), "," , "GSR data from Arduino");
			goToFrame(5);
		}
	}
	else if (paused == false && transitionState == TRANSITION_LOCK && currentFrameIndex == 7) {
		
		if (memoryPlayheadX < bounds.x+bounds.width) {
			// move the playhead in the mirror
			memoryPlayheadX += 3.0f;
		}
		else {
			// stop playing now
			memoryPlayheadX = bounds.x+bounds.width;
			transitionState = TRANSITION_READY;
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	// draw the background first
	ofSetColor(255);
	background.draw(0, 0);
	
	// draw main frame
	ofSetColor(255, 255, 255);
	frames[currentFrameIndex].draw(currentFramePosX, 0);
	
	// draw parts of the experience
	if (transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
		
		// in the experience
		ofSetColor(255);
		
		// use FTT to affect the diameter
		float dancerOffset = 0.0f;
		float spectatorOffset = 0.0f;
		
		if (currentPerspective == DANCER_PERSPECTIVE) {
			dancerOffset = fftVolume * 60.0f;
		}
		else if (currentPerspective == SPECTATOR_PERSPECTIVE) {
			spectatorOffset = fftVolume * 60.0f;
		}
		
		// draw the circles
		ofEllipse(dancerPosX, (ofGetHeight() * 0.5f),
				  dancerDiameter+dancerOffset, dancerDiameter+dancerOffset);
		ofEllipse(spectatorPosX, (ofGetHeight() * 0.5f),
				  spectatorDiameter+spectatorOffset, spectatorDiameter+spectatorOffset);
		
		// calculate the current time
		int currentTime = spectatorStory.getPositionMS() / 1000;
		int currentSeconds = currentTime % 60;
		currentTime /= 60;
		int currentMinutes = currentTime % 60;
		
		// draw the current time
		string s = nf(currentMinutes, 2) + ":" + nf(currentSeconds, 2) + "/" + nf(spectatorDurationMinutes,2) + ":" + nf(spectatorDurationSeconds,2);
		ofRectangle rect = futuraLight48.getStringBoundingBox(s, 0, 0);
		futuraLight48.drawString(s, (int)(ofGetWidth()*0.5f - (rect.width*0.5f) ), (ofGetHeight()*0.5f)+18.0f);
		
		// draw the voice labels
		rect = futuraBook18.getStringBoundingBox("Dancer", 0, 0);
		futuraBook18.drawString("Dancer", dancerPosX-(rect.width*0.5), (ofGetHeight()*0.5f) + ((dancerDiameter+dancerOffset)*0.5f) + 30.0f);
		rect = futuraBook18.getStringBoundingBox("Spectator", 0, 0);
		futuraBook18.drawString("Spectator", spectatorPosX-(rect.width*0.5f), (ofGetHeight()*0.5f) + ((spectatorDiameter+spectatorOffset)*0.5f) + 30.0f);
	}
	else if ( transitionState != TRANSITION_IN && transitionState != TRANSITION_OUT && currentFrameIndex == 7) {
		
		
		// draw the sensor graph
		ofSetLineWidth(2);
		sensorDataLine.draw();
		
		// draw the wafeforms
		ofSetColor(255, 255, 255, 30);
		dancerWaveform.draw(0, 0);
		spectatorWaveform.draw(0, 0);
		
		ofFill();
		ofSetColor(255, 255, 255,255);
		ofSetLineWidth(1.0f);
		
		ofRectangle ln;
		
		// draw the dancer lines
		for (int i = 0; i < dancerDataLine.size(); i++) {
//			ofRect(dancerDataLine[i]);
			ofSetColor(255, 255, 255,255);
			ln = dancerDataLine[i];			
			dancerWaveform.drawSubsection(ln.x, ln.y, ln.width, ln.height, ln.x, ln.y);
			
			// draw a divide line
			ofSetColor(255, 255, 255,30);
			ofLine(ln.x+0.5f, bounds.y, ln.x+0.5f, spectator.y+spectator.height);
		}
		
		// draw the spectator lines
		for (int i = 0; i < spectatorDataLine.size(); i++) {
//			ofRect(spectatorDataLine[i]);
			ofSetColor(255, 255, 255,255);
			ln = spectatorDataLine[i];
			spectatorWaveform.drawSubsection(ln.x, ln.y, ln.width, ln.height, ln.x, ln.y);
			
			// draw a divide line
			ofSetColor(255, 255, 255,30);
			ofLine(ln.x+0.5f, bounds.y, ln.x+0.5f, spectator.y+spectator.height);
		}
		
		// draw the reveal over the graph
		ofSetColor(255, 255, 255, 255);
		ofFill();
		background.drawSubsection(memoryPlayheadX, bounds.y-10,
								  bounds.x+bounds.width-memoryPlayheadX, spectator.y+spectator.height-bounds.y+20,
								  memoryPlayheadX, bounds.y-10);
		
		ofNoFill();
		
		// calculate the total length once (we don't need to keep re-calculating this)
		int totalTime = ofMap(memoryPlayheadX, bounds.x, bounds.x+bounds.width, 0, 185);//185 = TOTAL_AUDIO_LENGTH / 1000;
		int s = totalTime % 60;
		totalTime /= 60;
		int m = totalTime % 60;
		
		// draw the playhead
		ofSetColor(255, 255, 255, 255);
		ofLine(memoryPlayheadX+0.5f, bounds.y, memoryPlayheadX+0.5f, spectator.y+spectator.height);
		futuraBook18.drawString(nf(m,2)+":"+nf(s,2), memoryPlayheadX-27, spectator.y+spectator.height+20);
		
	}

	// draw the heading underline
	ofSetLineWidth(3);
	ofLine(underlineStart-80.0f, 70.5f, underlineEnd-80.0f, 70.5f);
	
	// draw the global helper text at the bottom of the screen
	ofSetLineWidth(1);
	
	if (transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
		// show the prompt at the bottom
		ofRectangle rect = futuraBook18.getStringBoundingBox(helperText[currentFrameIndex], ofGetWidth()*0.5f, ofGetHeight()-54);
		futuraBook18.drawString(helperText[currentFrameIndex], rect.x-(rect.width*0.5f), rect.y);
		ofLine(rect.x-(rect.width*0.5f), (int)rect.y+rect.height-13.5f, (rect.x-(rect.width*0.5f))+rect.width, (int)rect.y+rect.height-13.5f);
	}
	else if (transitionState == TRANSITION_READY) {
		// show the prompt at the bottom
		ofRectangle rect = futuraBook18.getStringBoundingBox(helperText[currentFrameIndex], ofGetWidth()*0.5f, ofGetHeight()-54);
		futuraBook18.drawString(helperText[currentFrameIndex], rect.x-(rect.width*0.5f), rect.y);
		ofLine(rect.x-(rect.width*0.5f), rect.y+rect.height-13.5f, (rect.x-(rect.width*0.5f))+rect.width, rect.y+rect.height-13.5f);
	}
	
	// draw prompt
	ofSetColor(255, 255, 255, framePromptAlpha);
	framePrompt.draw(0,0);
	
	// draw sensor preview graph
	ofNoFill();
	ofSetColor(255, 255, 255, ofMap(currentFrameIndex, 0, 4, 100, 0, true));
	ofSetLineWidth(3);
	ofBeginShape();
	float scale = (float)ofGetWidth() / (SENSOR_PREVIEW_SIZE-1);
	
	for (int i = 0; i < SENSOR_PREVIEW_SIZE; i++) {
		if ( sensorPreview[i] > 0 ) {
			ofVertex(i*scale, ofMap(sensorPreview[i], 0, 700, ofGetHeight() + 150, 150, true));
		}
	}
	ofEndShape();
	
	// debugging
	if (debugging == true) {
		if (sensorInUse == true) {
			ofSetColor(0, 255, 0);
		} else {
			ofSetColor(255, 0, 0);
		}
		ofDrawBitmapString( "SENSOR READING: " + ofToString(sensorValue) + " / PERSPECTIVE: " + ofToString(currentPerspective), 5, ofGetHeight()-5 );
		
		if (paused == true) {
			ofDrawBitmapString("RESETTING: " + ofToString(USER_TIMEOUT - (ofGetElapsedTimeMillis() - lastUserInteraction)), ofGetWidth()-200, ofGetHeight()-5);
		}
	}
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	
	// make sure the sensor IS being used
	if ( sensorInUse == true ) {
		
		if ( key == ' ' ) {
			
			if (currentFrameIndex < frames.size()-1 ) {
				goToFrame(currentFrameIndex+1);
			}
			else {
				goToFrame(2);
			}
		}
		else if ( key == OF_KEY_LEFT && transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
			
			// we are watching the main experience
			if ( currentPerspective == SPECTATOR_PERSPECTIVE) {
				
				currentPerspective = DANCER_PERSPECTIVE;
				
				dancerStory.setPan(0.0f);
				dancerStory.setVolume(HIGH_VOLUME);
				
				spectatorStory.setPan(1.0f);
				spectatorStory.setVolume(LOW_VOLUME);
				
				updatePerspectives();
			}
		}
		else if ( key == OF_KEY_RIGHT && transitionState == TRANSITION_LOCK && currentFrameIndex == 4 ) {
			
			// we are watching the main experience
			if (currentPerspective == DANCER_PERSPECTIVE) {
				
				currentPerspective = SPECTATOR_PERSPECTIVE;
				
				spectatorStory.setPan(0.0f);
				spectatorStory.setVolume(HIGH_VOLUME);
				
				dancerStory.setPan(-1.0f);
				dancerStory.setVolume(LOW_VOLUME);
				
				updatePerspectives();
			}
		}
	}
	
	// here we DONT care about the sensor readings
	if ( key == '0' ) {
		ofLogNotice() << "return to start" << endl;
		resetExperience();
	}
	else if ( key == 'd' ) {
		// toggle the debugging code
		debugging = !debugging;
	}
	else if ( key == 'f' ) {
		ofSetFullscreen(true);
	}
	else if ( key == OF_KEY_SHIFT ) {
		sensorInUse = true;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if ( key == OF_KEY_SHIFT ) {
		sensorInUse = false;
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
	ofLogNotice() << x << "/" << y;
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

//--------------------------------------------------------------
void testApp::setupArduino(const int & version) {
	
	// remove listener because we don't need it anymore
	ofRemoveListener(arduino.EInitialized, this, &testApp::setupArduino);
    
    // print firmware name and version to the console
    ofLogNotice() << arduino.getFirmwareName();
    ofLogNotice() << "firmata v" << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();
    
    // set pin D2 to digital input
    arduino.sendDigitalPinMode(BUTTON_PIN, ARD_INPUT);
	
    // set pin A0 to analog input
    arduino.sendAnalogPinReporting(SENSOR_PIN, ARD_ANALOG);
	
    // Listen for changes on the digital and analog pins
    ofAddListener(arduino.EDigitalPinChanged, this, &testApp::digitalPinChanged);
    ofAddListener(arduino.EAnalogPinChanged, this, &testApp::analogPinChanged);
}

//--------------------------------------------------------------
void testApp::digitalPinChanged(const int & pinNum) {
//    ofLogNotice() << "digital pin: " << pinNum << " = " << arduino.getDigital(pinNum) << endl;
	
	if (pinNum == BUTTON_PIN) {
		// set the sensorInUse flag if the button is pressed
		sensorInUse = arduino.getDigital(pinNum);
	}
}

//--------------------------------------------------------------
void testApp::analogPinChanged(const int & pinNum) {
//    ofLogNotice() << "analog pin: " << pinNum << " = " << arduino.getAnalog(pinNum) << endl;
	
	if (pinNum == SENSOR_PIN) {
		// set the sensorValue value
		sensorValue = arduino.getAnalog(pinNum);
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
void testApp::updatePerspectives() {
	
	// calculate the size and position of the spectator circle
	spectatorDiameterTarget = ofMap(spectatorStory.getPan(), 0.0f, 1.0f, 350.0f, 50.0f);
	spectatorPosXTarget = ofMap(spectatorStory.getPan(), 0.0f, 1.0f, ofGetWidth() * 0.5f, ofGetWidth() * 0.72f);
	
	// calculate the size and position of the dancer circle
	dancerDiameterTarget = ofMap(dancerStory.getPan(), -1.0f, 0.0f, 50, 350.0f);
	dancerPosXTarget = ofMap(dancerStory.getPan(), -1.0f, 0.0f, ofGetWidth() * 0.28f, ofGetWidth() * 0.5f);
}

//--------------------------------------------------------------
void testApp::goToFrame(int _nextFrame) {
	ofLogNotice() << "GOTO: " << _nextFrame;
	
	if (transitionState == TRANSITION_READY) {
		transitionState = TRANSITION_OUT;
		
		/*if (_nextFrame == 3) {
			// skip
			_nextFrame = 4;
		}
		else if (_nextFrame == 8 || _nextFrame == 9) {
			// loop
			resetExperience();
		}*/
		
		nextFrameIndex = _nextFrame;
	}
	else {
		cout << "woah dude, too fast!" << endl;
	}
}

//--------------------------------------------------------------
void testApp::pauseExperience() {
	ofLogNotice() << "PAUSE EXPERIENCE" << endl;
	
	// remember when this happened
	paused = true;
	lastUserInteraction = ofGetElapsedTimeMillis();
	
	// check if we're playing audio etc
	if (transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
		
		dancerStory.setPaused(true);
		spectatorStory.setPaused(true);
	}
}

//--------------------------------------------------------------
void testApp::resumeExperience() {
	ofLogNotice() << "RESUME EXPERIENCE" << endl;
	
	paused = false;
	
	// check if we're playing audio etc
	if (transitionState == TRANSITION_LOCK && currentFrameIndex == 4) {
		
		dancerStory.setPaused(false);
		spectatorStory.setPaused(false);
	}
}

//--------------------------------------------------------------
void testApp::resetExperience() {
	ofLogNotice() << "RESET EXPERIENCE" << endl;
	
	paused = false;
	transitionState = TRANSITION_READY;
	currentPerspective = DANCER_PERSPECTIVE;
	currentFrameIndex = 0;
	underlineStart = underlineTargets[currentFrameIndex][0];
	underlineEnd = underlineTargets[currentFrameIndex][1];
	dancerStory.stop();
	spectatorStory.stop();
	csv.clear();
	sensorDataLine.clear();
	dancerDataLine.clear();
	spectatorDataLine.clear();
}