#ifndef CAMERAFRAME_H
#define CAMERAFRAME_H


#include <stdlib.h>
#include <string>


/* Video frame formats */
enum Format {
	FORMAT_YUV,
	FORMAT_RGB,
	FORMAT_BGR,
	FORMAT_MJPG
};


/*
 * This class stores the video stream information such as the
 * frame dimensions, fps etc.
 */
class VideoInfo {

	public:

		/* default constructor */
		VideoInfo();

		/* Image dimension */
		int w, h;

		/* Frames per second */
		int fps;

		/* Video format */
		Format format;

		/* Device name, usually videoX */
		std::string devname;

		/* Save path */
		std::string save_path;

};



class CameraFrame {

	public:

		/*
		 * Constructor with parameters to initialise the object
		 */
		CameraFrame(int _w					= 0,
					int _h					= 0,
					int _bpp				= 0,
					unsigned char *_data	= NULL,
					size_t _sz				= 0,
					Format _format			= FORMAT_RGB,
					bool b_copy_data		= false,
					bool b_become_parent	= true);


		/*
		 * Copy constructor
		 */
		CameraFrame(const CameraFrame &orig);

		/*
		 * Assignment operator
		 */
		CameraFrame & operator= (const CameraFrame & other);

		/*
		 * Destructor
		 */
		virtual ~CameraFrame();

		/*
		 * Can be used when the same object is needed. The creation is faster
		 * when the size matches the old size
		 */
		void create(int _w, int _h, int _bpp, const unsigned char *_data, size_t _sz, Format _format);

		/*
		 * Release the dynamically allocated memory and reset the variables
		 */
		void release();


		int w;					// width
		int h;					// height
		unsigned char *data;	// data
		int bpp;				// bytes per pixel
		size_t sz;				// size in bytes of the image data
		Format format;			// frame format
		bool b_own_data;		// does this frame own the data

};


#endif

