//
//  AlternatingFramesPlayer.h
//  alternatingFrameVideos
//
//  Created by Oriol Ferrer Mesià on 04/02/13.
//
//

#ifndef __alternatingFrameVideos__AlternatingFramesPlayer__
#define __alternatingFrameVideos__AlternatingFramesPlayer__

#include "ofmain.h"
#include "ofxRemoteUI.h"
#include "ofxXmlSettings.h"


class AlternatingFramesPlayer{


public:

	AlternatingFramesPlayer();
	~AlternatingFramesPlayer();

	void setup(vector<string> fileNames, vector<int> offsets);

	void update();
	void advance();
	void draw();

	void setSegmentDuration(int frames); // doesnt include crossfade frames
	void setCrossFadeDuration(int frames);
	void setExitOnEnd(bool exit){ exitOnEnd = exit;};

	void allPlayersGoToFrame(int frame); //takes in account offset for u

	void saveSettings();
	void loadSettings();

	bool debug;

private:
	void nextPlayer();
	void goToFrameOffset(int player, int offset);

	vector<ofQTKitPlayer*> players;
	vector<int> offsets;

	int segmentFrames;
	int overlapFrames;
	int numPlayers;
	bool exitOnEnd;
	bool autoAdvance;

	//current indices
	int segment;
	int overlap;
	int player;
	int direction; // [1 .. -1]
	bool overlapping; // this runs the "mode", full cam src or crossFade between srcs
	int totalFrames;

	//
	float videoFrameDuration;
	float currentFrameDuration;
	float prevTime;
	float timeOverflow;

	//ui action
	

};

#endif /* defined(__alternatingFrameVideos__AlternatingFramesPlayer__) */
