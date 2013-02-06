//
//  AlternatingFramesPlayer.cpp
//  alternatingFrameVideos
//
//  Created by Oriol Ferrer Mesià on 04/02/13.
//
//

#include "AlternatingFramesPlayer.h"


AlternatingFramesPlayer::AlternatingFramesPlayer(){

	cout << "loading playslist and settings" << endl;
	vector<string> fileNames;
	vector<int> offs;

	ofxXmlSettings pl;
	pl.loadFile("playlist.xml");
	int numV = pl.getNumTags("video");
	for(int i = 0 ; i < numV; i++){
		string file = pl.getValue("video", "NULL", i);
		int off = pl.getValue("offset", 0, i);
		cout << "playlist.xml includes " << file << endl;
		fileNames.push_back(file);
		offs.push_back(off);
	}

	setup(fileNames, offs);
	loadSettings();
}

AlternatingFramesPlayer::~AlternatingFramesPlayer(){

}

void AlternatingFramesPlayer::loadSettings(){

	ofxXmlSettings settings;
	settings.loadFile("settings.xml");
	segmentFrames = settings.getValue("segmentFrames", 3);
	overlapFrames = settings.getValue("overlapFrames", 0);
	exitOnEnd = settings.getValue("exitOnEnd", false);
	autoAdvance = settings.getValue("autoAdvance", false);
	debug = settings.getValue("debug", true);

}

void AlternatingFramesPlayer::saveSettings(){

	cout << "saving settings" << endl;
	ofxXmlSettings settings;
	settings.loadFile("settings.xml");
	settings.clear();
	settings.setValue("segmentFrames", segmentFrames);
	settings.setValue("overlapFrames", overlapFrames);
	settings.setValue("exitOnEnd", exitOnEnd);
	settings.setValue("autoAdvance", autoAdvance);
	settings.setValue("debug", debug);

	settings.saveFile("settings.xml");
}

void AlternatingFramesPlayer::allPlayersGoToFrame(int frame){ //takes in account offset for u

	for (int i = 0;i < players.size(); i++){
		goToFrameOffset(i, frame);
	}
}

void AlternatingFramesPlayer::setup(vector<string> fileNames, vector<int> offs){

	players.clear();
	offsets.clear();
	
	for(int i = 0; i < fileNames.size(); i++){

		ofFile f;
		bool exists = f.doesFileExist(fileNames[i]);
		if (exists){
			cout << "loading " << fileNames[i] << " for camera " << ofToString(i) << endl;
			ofQTKitPlayer * p = new ofQTKitPlayer();
			p->setPixelFormat(OF_PIXELS_RGB);

			// ofQTKitDecodeMode decodeMode = OF_QTKIT_DECODE_TEXTURE_ONLY;
			// Pixels and texture together is faster than PIXEL_ONLY and manually uploaded textures.
			ofQTKitDecodeMode decodeMode = OF_QTKIT_DECODE_TEXTURE_ONLY;
			bool ok = p->loadMovie(fileNames[i], decodeMode);
			if (!ok) cout << "Error at loadMovie() " << fileNames[i] << endl;
			cout << "movie clip " << fileNames[i] <<  " has " << p->getTotalNumFrames() << " frames, and a duration of " << ofToString(p->getDuration()) << endl;
			p->setSynchronousSeeking(false);
			p->setPaused(true);
			p->setFrame(offs[i]);
			players.push_back(p);
			offsets.push_back(offs[i]);
			if (i == 0){
				ofSetWindowShape(p->getWidth(), p->getHeight());
				videoFrameDuration = p->getDuration() / p->getTotalNumFrames();
			}
		}else{
			cout << "ERROR! Can't load " << fileNames[i] << " for camera " << ofToString(i) << endl;
			ofExit(-1);
		}
	}

	numPlayers = fileNames.size();

	exitOnEnd = false;

	segment = 0;
	overlap = 0;
	player = 0;
	direction = 1;
	overlapping = false;
	totalFrames = 0;

	autoAdvance = false;
	debug = false;

	OFX_REMOTEUI_SERVER_SETUP();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(segmentFrames, 1, 50);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(overlapFrames, 0, 50);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(autoAdvance);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(debug);

	prevTime = 0.0f;
	currentFrameDuration = 0.0f;
	timeOverflow = 0.0f;
}

void AlternatingFramesPlayer::goToFrameOffset(int playerID, int off){
	players[playerID]->setFrame( off + offsets[playerID] );
}

void AlternatingFramesPlayer::update(){

	OFX_REMOTEUI_SERVER_UPDATE(0.016666);

	if (autoAdvance){
		float t = ofGetElapsedTimef();
		float thisFrame = t - prevTime;
		//cout << ofToString(thisFrame) << endl;
		currentFrameDuration += thisFrame;
		if (currentFrameDuration > videoFrameDuration + timeOverflow){
			//cout << "advance! currentFrameDuration :" << ofToString(currentFrameDuration) << "  frameDuration: " << ofToString(videoFrameDuration) << endl;
			advance();
			timeOverflow = 0.8 * timeOverflow + 0.2 * ( videoFrameDuration - currentFrameDuration );
			currentFrameDuration = 0.0f;
		}
		prevTime = t;
	}
	
	bool didEnd = false;
	for (int i = 0;i < players.size(); i++){
		players[i]->update();
		if (players[i]->getCurrentFrame() == players[i]->getTotalNumFrames()){
			cout << "video " << ofToString(i) << "ended" << endl;
			didEnd = true;
		}
	}

	if (didEnd){
		if (exitOnEnd){
			cout << "video ended... exiting" << endl;
			ofExit(0);
		}else{
			for (int i = 0;i < players.size(); i++){
				//players[i]->firstFrame();
				goToFrameOffset(i, 0);
			}
			totalFrames = 0;
		}
	}
}

void AlternatingFramesPlayer::nextPlayer(){
	segment = 0;
	overlapping = false;
	player+= direction;

	bool needsToTurn = (player == 0 && direction == -1) || (player == numPlayers -1 && direction == 1);
	if ( needsToTurn ){	 // change direction!
		direction *= -1;
	}
}

void AlternatingFramesPlayer::advance(){

	for (int i = 0;i < players.size(); i++){
		players[i]->nextFrame();
	}

	bool isOnEdge = player == 0 || player == numPlayers -1;
	bool needsToTurn = (player == 0 && direction == -1) || (player == numPlayers -1 && direction == 1);
	if (overlapping){ // we are in the crossFade-phase
		if (overlap >= overlapFrames){ // we ended the blending
			nextPlayer();
		}
		overlap ++;
	}else{
		int numSegmentFrames = (needsToTurn ? segmentFrames/2 : segmentFrames);
		if (segment >= numSegmentFrames){
			if (overlapFrames > 0){
				overlapping = true;
				overlap = 0;
			}else{
				nextPlayer();
			}
		}
		segment++;
	}


	totalFrames++;
}


void AlternatingFramesPlayer::draw(){

	ofQTKitPlayer * current = players[player];
	ofQTKitPlayer * future = players[player + direction];

	ofSetColor(255);
	current->draw(0, 0, ofGetWidth(), ofGetHeight());
	if (overlapping){
		float percent = (overlap + 1) / (float)(overlapFrames + 1 );
		ofSetColor(255, 255 * percent);
		future->draw(0, 0, ofGetWidth(), ofGetHeight());
	}

	if(debug){
		ofSetColor(255, 0, 0);

		string pOffs;
		for(int i = 0; i < players.size(); i++){
			pOffs += "\nplayer " + ofToString(i) + ": " + ofToString(players[i]->getCurrentFrame());
		}

		ofDrawBitmapStringHighlight("totalFrames: " + ofToString(totalFrames) + "\n" +
									"player: " + ofToString(player) + "\n" +
									"segment: " + ofToString(segment) + "\n" +
									"overlapping: " + ofToString(overlapping) + "\n" +
									"overlap: " + ofToString(overlap) + "\n" +
									"direction: " + ofToString(direction) + "\n" +
									pOffs,
									20,
									20);
	}

}


void AlternatingFramesPlayer::setSegmentDuration(int frames){
	segmentFrames = frames;
};

void AlternatingFramesPlayer::setCrossFadeDuration(int frames){
	overlapFrames = frames;
};
