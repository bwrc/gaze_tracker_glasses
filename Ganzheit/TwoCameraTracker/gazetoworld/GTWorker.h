#ifndef GTWORKER_H
#define GTWORKER_H


#include "JPEGWorker.h"
#include "GazeTracker.h"
#include "SceneMapper.h"



class GTWorker : public JPEGWorker {

	public:

		GTWorker();

		void setTracker(gt::GazeTracker *_tracker, SceneMapper *_mapper);

	private:

		CameraFrame *process(CameraFrame *img_compr);
		gt::GazeTracker *tracker;
		SceneMapper *mapper;

};


#endif

