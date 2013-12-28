#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	ofSetFrameRate(30);
	ofSetLineWidth(1.0f);
	
	// load the fonts
	ofTrueTypeFont::setGlobalDpi(72);
	futuraBook14.loadFont("fonts/FuturaStd-Book.ttf", 14);
	futuraBook14.setSpaceSize(0.5f);
	
	background.loadImage("images/background.png");

	filename = "MyDataFile.csv";
	csv.loadFile( ofToDataPath(filename) );
	cout << "loaded " << csv.numRows << " rows" << endl;
	
	bounds = ofRectangle(100, 200, ofGetWidth()-200, 150);
	dancer = ofRectangle(100, bounds.y+bounds.height, bounds.width, 50);
	spectator = ofRectangle(100, dancer.y+dancer.height, bounds.width, 50);
	
	minSensorValue = 1023;
	maxSensorValue = 0;
	showInterval = false;
	showIntervalX = 0.0f;
	
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
				dancerDataLine.push_back( ofRectangle(startX, dancer.y, mappedPosX-startX, 50) );
			}
			else if (prevPerspective == SPECTATOR_PERSPECTIVE) {
				spectatorDataLine.push_back( ofRectangle(startX, spectator.y, mappedPosX-startX, 50) );
			}
			
			// we've switched so remember the start
			startX = mappedPosX;
		}
		
		// remember the current for the next loops
		prevPerspective = dataPerspective;
	}
	
	// and once the loop's completed, we should close the final rect
	if (prevPerspective == DANCER_PERSPECTIVE) {
		dancerDataLine.push_back( ofRectangle(startX, dancer.y, mappedPosX-startX, 50) );
	}
	else if (prevPerspective == SPECTATOR_PERSPECTIVE) {
		spectatorDataLine.push_back( ofRectangle(startX, spectator.y, mappedPosX-startX, 50) );
	}
}

//--------------------------------------------------------------
void testApp::update(){
	
	// calculate the sensor readings
	if (showInterval == true) {
		sPosition = ofMap(showIntervalX, bounds.x, bounds.x+bounds.width, 0.0f, 1.0f);
		sRow = ofMap(showIntervalX, bounds.x, bounds.x+bounds.width, 0, csv.numRows);
		
		int time = csv.getInt(sRow, TIME_COLUMN) / 1000;
		int tSeconds = time % 60;
		time /= 60;
		int tMinutes = time % 60;
		
		sTime = nf(tMinutes, 2) + ":" + nf(tSeconds, 2);
		sSensor = csv.getInt(sRow, SENSOR_COLUMN);
		sPerspective = (csv.getInt(sRow, PERSPECTIVE_COLUMN) == DANCER_PERSPECTIVE) ? "Dancer" : "Spectator";
	}

}

//--------------------------------------------------------------
void testApp::draw() {
	
	// draw the background
	ofSetColor(255);
	background.draw(0, 0);
	
	// draw footer
	ofDrawBitmapString("Loaded: " + filename+ " (" + ofToString(csv.numRows) + " rows)", 20.0f, ofGetHeight()-40.0f);
	ofDrawBitmapString("% = percentage / x = index / t = time / s = sensor / p = perspective", 20.0f, ofGetHeight()-20.0f);
	
	// draw the bounds background
	ofPushMatrix();
	ofTranslate(0.5f, 0.5f);
	ofSetColor(255, 255, 255, 30);
	ofRect(bounds);
	ofRect(dancer);
	ofRect(spectator);
	ofPopMatrix();
	
	// draw the sensor graph
	ofSetColor(255, 255, 255, 255);
	sensorDataLine.draw();	
	
	ofFill();
	
	// draw the dancer lines
	for (int i = 0; i < dancerDataLine.size(); i++) {
		
		if (dancerDataLine[i].inside(showIntervalX, dancer.y+25)) {
			ofSetColor(255, 112, 112);
		}
		else {
			ofSetColor(255);
		}
		ofRect(dancerDataLine[i]);
	}
	
	// draw the spectator lines
	for (int i = 0; i < spectatorDataLine.size(); i++) {
		if (spectatorDataLine[i].inside(showIntervalX, spectator.y+25)) {
			ofSetColor(255, 112, 112);
		}
		else {
			ofSetColor(255);
		}
		ofRect(spectatorDataLine[i]);
	}
	
	ofNoFill();
	ofSetColor(255);
	
	// draw the graph labels
	futuraBook14.drawString(ofToString(maxSensorValue), bounds.x+bounds.width+10.0f, bounds.y+5);
	futuraBook14.drawString(ofToString(minSensorValue), bounds.x+bounds.width+10.0f, bounds.y+bounds.height+5	);
	
	if (showInterval == true) {
		ofPushStyle();
		ofSetColor(255, 112, 112);
		ofLine(showIntervalX+0.5f, bounds.y-10, showIntervalX+0.5f, spectator.y+spectator.height+86);
		ofFill();
		ofEllipse(showIntervalX, ofMap(sSensor, minSensorValue, maxSensorValue,
									   bounds.y+bounds.height, bounds.y),
				  6.0f, 6.0f);
		ofNoFill();
		
		ofPushMatrix();
		ofTranslate(10.5f, 10.0f);
		
		futuraBook14.drawString("%: " + ofToString(sPosition), showIntervalX, spectator.y+spectator.height+15);
		futuraBook14.drawString("X: " + ofToString(sRow), showIntervalX, spectator.y+spectator.height+30);
		futuraBook14.drawString("T: " + ofToString(sTime), showIntervalX, spectator.y+spectator.height+45);
		futuraBook14.drawString("S: " + ofToString(sSensor), showIntervalX, spectator.y+spectator.height+60);
		futuraBook14.drawString("P: " + ofToString(sPerspective), showIntervalX, spectator.y+spectator.height+75);

		ofPopMatrix();
		ofPopStyle();
	}
	
	
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	if(bounds.inside(x, y) || dancer.inside(x, y) || spectator.inside(x, y)) {
		showInterval = true;
		showIntervalX = x;
	}
	else {
		showInterval = false;
	}
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

//--------------------------------------------------------------
string testApp::nf(int number, int digits) {
	string format = "%0" + ofToString(digits) + "d";
    char buffer[100];
    sprintf(buffer, format.c_str(), number);
    return (string)buffer;
}
