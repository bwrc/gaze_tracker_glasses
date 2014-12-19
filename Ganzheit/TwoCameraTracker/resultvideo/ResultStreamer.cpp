#include "ResultStreamer.h"
#include <sys/stat.h>
#include <iostream>



ResultStreamer::ResultStreamer() {

    framePos = 0;

}


bool ResultStreamer::init(const std::string &_folder) {

    framePos = 0;
    prevDataPacket = ResultData();

    parentDir = _folder;


    // see that the folder exists
    if(!ResultStreamer::exists(parentDir)) {

        return false;

    }

    if(!eyeStream.init(parentDir, std::string("camera1.mjpg"))) {

        return false;

    }

    if(!sceneStream.init(parentDir, "camera2.mjpg")) {

        return false;

    }

	if(!resStream.init(parentDir, "results.res")) {

		return false;
	}

    return true;

}


bool ResultStreamer::reset() {

    return init(parentDir);

}


int ResultStreamer::getFourCC() {
    return static_cast<int>(sceneStream.cap.get(CV_CAP_PROP_FOURCC));
}


cv::Size ResultStreamer::getDim() {

    return cv::Size((int) sceneStream.cap.get(CV_CAP_PROP_FRAME_WIDTH),
                    (int) sceneStream.cap.get(CV_CAP_PROP_FRAME_HEIGHT));

}


int ResultStreamer::getFps() {

    return sceneStream.cap.get(CV_CAP_PROP_FPS);

}


int ResultStreamer::get(cv::Mat &imgEye, cv::Mat &imgScene, ResultData &data) {

    data.clear();

    bool bSceneStreamOk = sceneStream.getNextFrame(imgScene);
    bool bEyeStreamOk   = eyeStream.getNextFrame(imgEye);

    // if just one of them is ok, there is an error
    if((bSceneStreamOk && !bEyeStreamOk) || (!bSceneStreamOk && bEyeStreamOk)) {
        return STREAM_ERROR;
    }

    if(!bSceneStreamOk && !bEyeStreamOk) {
        return STREAM_FINISHED;
    }


    // get new data only if the current frame position exceeds the previous data id
    if(prevDataPacket.id < framePos || framePos == 0) {

        bool bResStreamOk = resStream.getNextDataPacket(prevDataPacket);
        if(!bResStreamOk) {

            return STREAM_ERROR;

        }

    }

    if(prevDataPacket.id == framePos) {
        data = prevDataPacket;
    }

    ++framePos;

    return STREAM_OK;

}


bool ResultStreamer::exists(const std::string &dir) {

    struct stat myStat;
    if((stat(dir.c_str(), &myStat) == 0) && (((myStat.st_mode) & S_IFMT) == S_IFDIR)) {
        return true;
    }

    return false;

}




/******************************************************************
 * DataStream class
 ******************************************************************/

bool DataStream::init(const std::string &folder, const std::string &_fileName) {

    folderID = -1;

    // the parent folder, the one that containes part0, part1...
    parentDir = folder;

    // the file name in each partX, e.g. camera1.mjpg
    fileName = _fileName;

    return true;

}


std::string DataStream::getNextFileName() {

    // increase the id and create create a new file name
    ++folderID;

    std::stringstream ss;
    ss << parentDir << "part" << folderID << "/" << fileName;

    return ss.str();

}



/******************************************************************
 * VideoStream class, extends DataStream
 ******************************************************************/

bool VideoStream::init(const std::string &folder, const std::string &_fileName) {

    // call parent function
    if(!DataStream::init(folder, _fileName)) {
        return false;
    }

    std::string path = getNextFileName();

    cap.release();
    cap.open(path);

    return cap.isOpened();

}


bool VideoStream::getNextFrame(cv::Mat &frame) {

    cap >> frame;

    // if the frame is empty, change to a new file, if exists, and try again
    if(frame.empty()) {

        std::string path = getNextFileName();
        cap.release();
        cap.open(path);

        if(!cap.isOpened()) {

            return false;

        }

        cap >> frame;

    }


    return !frame.empty();

}




/******************************************************************
 * ResultStream class, extends DataStream
 ******************************************************************/


bool ResultStream::init(const std::string &folder, const std::string &_fileName) {

    if(!DataStream::init(folder, _fileName)) {
        return false;
    }

    resIn.close();
    resIn.clear();

    std::string path = getNextFileName();
    resIn.open(path.c_str(), std::ifstream::binary);
    resIn.seekg(0);

    return resIn.is_open();

}


bool ResultStream::getNextDataPacket(ResultData &data) {

	int32_t packetSz;
	resIn.read((char *)&packetSz, 4);

	int nRead = resIn.gcount();
	if(nRead != 4) {

        // nRead can be zero only if eof() was reached, otherwise there is a seious error
        if(resIn.eof()) {

            resIn.close();
            resIn.clear();

            std::string path = getNextFileName();
            resIn.open(path.c_str());
            resIn.seekg(0);

            if(!resIn.is_open()) {

                printf("ResultStream::getNextDataPacket(): Error opening the next file \"%s\"\n", path.c_str());
                return false;

            }


            resIn.read((char *)&packetSz, 4);

            int nRead = resIn.gcount();
            if(nRead != 4) {
                printf("ResultStream::getNextDataPacket(): could not read packet size in new file\n");
                return false;
            }

        }
        else { // nRead != 4 although eof() was not reached
            printf("ResultStream::getNextDataPacket(): could not read packet size\n");
            return false;
        }

	}


	const char *tmp = (char *)&packetSz;
	packetSz = (tmp[0] & 0x000000FF)       |
			   (tmp[1] & 0x000000FF) << 8  |
			   (tmp[2] & 0x000000FF) << 16 |
			   (tmp[3] & 0x000000FF) << 24;

	if(packetSz < BinaryResultParser::MIN_BYTES) {
        printf("ResultStream::getNextDataPacket(): packet size too small\n");
		return false;
	}

	std::vector<char> packet(packetSz);
	memcpy(packet.data(), tmp, 4);

	// read the rest
	resIn.read(packet.data() + 4, packet.size() - 4);

	nRead = resIn.gcount();
	if(nRead != (int)packet.size() - 4) {
        printf("ResultStream::getNextDataPacket(): could not read the rest\n");
		return false;
	}


	bool res = BinaryResultParser::parsePacket(packet.data(), packet.size(), data);

	if(!res) {
		printf("BinaryResultParser::parsePacket() failed\n");
		return false;
	}

    return true;

}

