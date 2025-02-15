/**
 * Copyright (C) ARM Limited 2010-2012. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SESSION_DATA_H
#define SESSION_DATA_H

#define MAX_PERFORMANCE_COUNTERS	50
#define MAX_STRING_LEN				80
#define MAX_DESCRIPTION_LEN			400

#define PROTOCOL_VERSION	8
#define PROTOCOL_DEV		1000	// Differentiates development versions (timestamp) from release versions

struct ImageLinkList {
	char *path;
	struct ImageLinkList *next;
};

class SessionData {
public:
	SessionData();
	~SessionData();
	void initialize();
	void initializeCounters();
	void parseSessionXML(char* xmlString);

	char mCoreName[MAX_STRING_LEN];
	struct ImageLinkList *images;
	char* configurationXMLPath;
	char* target_path;
	char* apcDir;
	char* title;

	bool mWaitingOnCommand;
	bool mSessionIsActive;
	bool mLocalCapture;
	bool mOneShot;		// halt processing of the driver data until profiling is complete or the buffer is filled
	
	int mBacktraceDepth;
	int mTotalBufferSize;	// approximate number of MB to use for the entire collection buffer, the actual amount is a multiple based on a buffer size retrieved from the driver
	int mSampleRate;
	int mDuration;
	int mCores;
	int mBytes;

	// PMU Counters
	char mPerfCounterType[MAX_PERFORMANCE_COUNTERS][MAX_STRING_LEN];
	char mPerfCounterTitle[MAX_PERFORMANCE_COUNTERS][MAX_STRING_LEN];
	char mPerfCounterName[MAX_PERFORMANCE_COUNTERS][MAX_STRING_LEN];
	char mPerfCounterDescription[MAX_PERFORMANCE_COUNTERS][MAX_DESCRIPTION_LEN];
	int mPerfCounterEnabled[MAX_PERFORMANCE_COUNTERS];
	int mPerfCounterEvent[MAX_PERFORMANCE_COUNTERS];
	int mPerfCounterColor[MAX_PERFORMANCE_COUNTERS];
	int mPerfCounterCount[MAX_PERFORMANCE_COUNTERS];
	int mPerfCounterKey[MAX_PERFORMANCE_COUNTERS];
	bool mPerfCounterPerCPU[MAX_PERFORMANCE_COUNTERS];
	bool mPerfCounterEBSCapable[MAX_PERFORMANCE_COUNTERS];
	char mPerfCounterOperation[MAX_PERFORMANCE_COUNTERS][MAX_STRING_LEN];
};

extern SessionData* gSessionData;

#endif // SESSION_DATA_H
