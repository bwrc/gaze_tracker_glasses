/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <arto.merilainen@ttl.fi> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <list>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <iostream>

#include "CaptureDevice.h"


using namespace std;


static void getImageDim(GstBuffer *buffer, int &w, int &h) {

    GstCaps* caps = gst_buffer_get_caps(buffer);
    GstVideoFormat format;
    if(!gst_video_format_parse_caps(caps, &format, &w, &h)) {
        gst_caps_unref(caps);
    }

}



/*******************************************************************************
 * bus_call() - Bus call handler.
 *
 * This function implements a minimalist mechanism for tracking bugs (=printf)
 * for the code. The function causing the failure detects the fault by
 * checking the state of the pipeline.
 ******************************************************************************/

static GstBusSyncReply bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
	switch (GST_MESSAGE_TYPE(msg)) {

		case GST_MESSAGE_EOS:
			g_message("End-of-stream");
			break;

		case GST_MESSAGE_ERROR:
			GError *err;
			gst_message_parse_error(msg, &err, NULL);
			std::cout << err->message << std::endl;
			g_error_free(err);

			break;

		default:
			break;
	}

	return (GST_BUS_PASS);
}


/*******************************************************************************
 * frameGrabbed_public - Fakesink callback
 *
 * This function is called when a buffer is received from the camera. The
 * function Performs sanity checks and calls FrameReceiver::frameReceived()
 ******************************************************************************/
static gboolean frameGrabbed_public(GstElement * element,
									GstBuffer * buffer,
									GstPad* pad,
									gpointer ptr) 
{

	// get the user data
	CBData *cbdata					= (CBData *)ptr;
	CaptureDevice *cap_dev			= cbdata->cap_dev;
	int id							= cap_dev->getID();
	const VideoInfo &info			= cbdata->info;
	FrameReceiver *receiver			= cap_dev->receiver;


	// get the buffer size in bytes
	guint sz = buffer->size;

	// get the image size
	int w, h;
	getImageDim(buffer, w, h);

	// get the target size, i.e. what has been requested
	const int FRAME_W = info.w;
	const int FRAME_H = info.h;

	// return if the size is incorrect
	if(w != FRAME_W || h != FRAME_H) {

		std::cout	<< "incorrect image size for " << id
					<< " (" << w << "," << h << "): size " << sz << std::endl;

		return FALSE;

	}

	// video format
	const Format format = info.format;


	// FIXME: remove the hat constant and ask the value from the buffer
	int bpp = 3;

	// get a pointer to the image data
	unsigned char *data = buffer->data;

	// create a header for the data. Does not copy data.
	CameraFrame frame(w,
					  h,
					  bpp,
					  data,
					  sz,
					  format,
					  false,	// copy data
					  false);	// do not become parent, i.e. do not destroy data in destructor

	// call the frame receiver
	receiver->frameReceived(&frame, id);

	// everythin is ok
	return TRUE;

}


CaptureDevice::CaptureDevice() {

	this->pipeline = NULL;

	receiver = NULL;

}


CaptureDevice::~CaptureDevice() {

	this->deletePipeline();

}



/*******************************************************************************
 * CaptureDevice::init - Initialize the capture device
 *
 * This function initializes the current capture device to use the given
 * resolution, framerate and format. The original pipeline is destroyed and a
 * new pipeline is created.
 ******************************************************************************/

bool CaptureDevice::init(const std::string &devpath,
						 const int _width,
						 const int _height,
						 const int _framerate,
						 const Format _format,
						 FrameReceiver *_receiver,
						 int _id)
{

	receiver	= _receiver;
	id			= _id;


	this->deletePipeline();

	this->pipeline = gst_pipeline_new("videograbber");

	// Set the callback
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
	gst_bus_set_sync_handler(bus, (GstBusSyncHandler)bus_call, this);
	gst_object_unref(bus);

	// Add the video source
	GstElement * camera = gst_element_factory_make ("v4l2src", NULL);
	g_object_set(G_OBJECT(camera), "device", (char *)devpath.c_str(),
			(char *)NULL);

	// Add the filter
	GstElement * filter0 = gst_element_factory_make ("capsfilter", NULL);
	{
		char * format_c;
		switch(_format) {
			case FORMAT_YUV: {
				format_c = (char *)"video/x-raw-yuv";
				break;
			}

			case FORMAT_RGB: {
				format_c = (char *)"video/x-raw-rgb";
				break;
			}

			case FORMAT_BGR: {
				format_c = (char *)"video/x-raw-bgr";
				break;
			}

			case FORMAT_MJPG: {
				format_c = (char *)"image/jpeg";
				break;
			}

			default:
				gst_object_unref(camera);
				gst_object_unref(G_OBJECT(filter0));
				gst_object_unref(GST_OBJECT(pipeline));
				return false;
		}

		GstCaps *filtercaps = gst_caps_new_simple (format_c,
			"width", G_TYPE_INT,  _width,
			"height", G_TYPE_INT, _height,
    			"framerate", GST_TYPE_FRACTION, _framerate, 1,
			(char *)NULL);

		g_object_set(G_OBJECT(filter0), "caps",  filtercaps, (char *)NULL);
		gst_caps_unref (filtercaps);
	}


	// Queue for holding the data
	GstElement * queue = gst_element_factory_make ("queue", (char *)NULL);

	// Callback
	GstElement * fakesink = gst_element_factory_make("fakesink", (char *)NULL);

	// Add the elements
	gst_bin_add_many(GST_BIN (this->pipeline), camera, filter0, queue, fakesink, (char *)NULL);


	/**************************************************************
	 * Populate the VideoInfo instance
	 **************************************************************/
	video_info.w			= _width;
	video_info.h			= _height;
	video_info.fps			= _framerate;
	video_info.format		= _format;
	video_info.devname		= devpath;	/* Device name, usually videoX */
	video_info.save_path	= std::string("test.avi");	// not used currently


	/**************************************************************
	 * Populate the CBData instance
	 **************************************************************/
	cbdata.cap_dev	= this;
	cbdata.info		= video_info;


	// Set properties of the fakesink element
	g_object_set (G_OBJECT (fakesink), "signal-handoffs", TRUE, (char *)NULL);
	g_signal_connect(fakesink, "handoff", G_CALLBACK (frameGrabbed_public), gpointer(&cbdata));

	// Finally, link elements together
	gst_element_link_many(camera, filter0, queue, fakesink, (char *)NULL);




	/***********************************************************************
	 * Try starting the pipeline
	 **********************************************************************/

	GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PAUSED);
	if(ret == GST_STATE_CHANGE_FAILURE) {
		deletePipeline();
        printf("CaptureDevice::init(): Could not pause stream\n");
		return false;
	}

	GstState state, pending;
	gst_element_get_state(GST_ELEMENT(this->pipeline), &state, &pending, GST_SECOND);
	if(state != GST_STATE_PAUSED) {
        printf("CaptureDevice::init() (2): Could not pause stream\n");
		this->deletePipeline();
		return false;
	}

	/***********************************************************************
	 * Yep. It should work. Now start it for real
	 **********************************************************************/

    ret = gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PLAYING);
    if(ret != GST_STATE_CHANGE_FAILURE) {

        GstState state, pending;
        do {

            ret = gst_element_get_state(GST_ELEMENT(this->pipeline), &state, &pending, GST_SECOND);

        } while(ret != GST_STATE_CHANGE_FAILURE && state != GST_STATE_PLAYING);

    }
    else {
        return false;
    }


	gst_element_get_state(GST_ELEMENT(this->pipeline), &state, &pending, GST_SECOND);
	if(state != GST_STATE_PLAYING) {
        printf("CaptureDevice::init(): Coult not play stream\n");
		this->deletePipeline();
		return false;
	}

	return true;
}

/*******************************************************************************
 * CaptureDevice::deletePipeline - Delete the current pipeline
 ******************************************************************************/

void CaptureDevice::deletePipeline() {

	if(this->pipeline == NULL) {
		return;
	}


	{/* Pause the pipeline */

		GstStateChangeReturn ret =
			gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);

		if(ret != GST_STATE_CHANGE_FAILURE) {

			GstState state, pending;
			do {

				gst_element_get_state(GST_ELEMENT(this->pipeline), &state, &pending, GST_SECOND);

			} while(state != GST_STATE_PAUSED);

		}

	}


	{/* set the pipeline to NULL */

		GstStateChangeReturn ret =
			gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);

		if(ret != GST_STATE_CHANGE_FAILURE) {

			GstState state, pending;
			do {
				gst_element_get_state(GST_ELEMENT(this->pipeline), &state, &pending, GST_SECOND);
			} while(state != GST_STATE_NULL) ;

		}

	}


	gst_object_unref(GST_OBJECT(pipeline));
	this->pipeline = NULL;
}

