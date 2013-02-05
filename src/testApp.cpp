#include "testApp.h"

// High Performance OS X Video Playback Example
//--------------------------------------------------------------
// This example shows how to use the OS X platform specific
// ofQTKitPlayer on its own without the cross platform
// ofVideoPlayer wrapper.  Apps you write in this way won't be
// cross platform, but can be faster for HD videos .
//--------------------------------------------------------------

void testApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofSetFrameRate(50);

}

void testApp::exit(){
	p.saveSettings();
	OFX_REMOTEUI_SERVER_CLOSE();
}

//--------------------------------------------------------------
void testApp::update(){

	p.update();
}

//--------------------------------------------------------------
void testApp::draw(){

	p.draw();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if(key == ' '){
		p.allPlayersGoToFrame(0);
	}else{
		p.advance();
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
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
