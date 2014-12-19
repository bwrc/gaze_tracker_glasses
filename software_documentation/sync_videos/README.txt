Streaming and synchronising two USB video streams


*************************************************************
* 1. Overview
*************************************************************

USB cameras are widely used and are good for purposes such as video conferences and for recording video which does not have to meet high quality criteria. Recently USB cameras have also found their place in computer vision applications such as augmented reality, robotics etc. The popularity of USB cameras arises from the good quality-price ratio.

USB cameras are also configurable to an extent defined by the manufacturers. Configurable parameters are usually:

    - frame rate
    - exposure time
    - focus, auto or manual
    - resolution
    - output format
    - brightness
    - contrast
    - saturation
    - sharpness
    - pan
    - tilt



*************************************************************
* 2. Setup
*************************************************************

2.1 Hardware and camera configuration parameters

In our setup there are two Microsoft LifeCam HD-5000 USB cameras. This USB camera supports a resolution up to 1280x720. The camera is also capable of outputting motion JPEG, MPJEG-compressed frames at the top framerate of 30 fps. The exposure time is set to a constant value to enable for a constant framerate, which is crucial in our application. The focus is also set to a fixed value, so that the camera parameters do no change.


2.2 Software

Capturing video stream from a camera is quite straight forward. The limiting factor for the number of simultaneously streamed devices is the maximum transmission rate of the USB bus. The bus' transmission rate required by a camera is dictated by the requested fps, (the) resolution and (the) video format.

In our application gstreamer is used for capturing video data from the cameras. The goal is to collect frames from the cameras and write them into their individual video files. In addition, the frames need to be processed and sent to a Graphical User Interface (GUI), which shows the contents of both frames as well as the results obtained in the processing step.

There are a number of procedures and design rules that are needed in order for the application to be able to perform the described operations. These design rules will be discussed in the following subsections.


2.2.1 Synchronising the camera streams

From the application point of view, a unit of processable data is a pair of frames, a frame from each camera. These frames must contain data collected as closely in time as possible. The cameras cannot be assumed to start streaming simultaneously. In other words, one camera may start streaming data earlier than the other. As the other camera starts delivering frames too, there are two cameras streaming at a fixed fps. Therefore, the time difference between the frames can be anything from 0 seconds to 1 / fps seconds. There is a possibility that a camera or the streamer interface might not be able to maintain the requested fps. In this case the frames from the other camera must be dropped as well.

For each camera there is an instance of class SimpleCapture. This class has a buffer where frames from the gstreamer will be copied. A call to grabFrame() gets the oldest frame from the buffer. If grabFrame() is not called as fast as or faster than the buffer gets filled, frame drops occur. In an effort to synchronise the cameras, grabFrame() will be called for both streams and the returned frames are considered synchronised. There, however, might be an offset of buffSz / fps seconds, i.e. the larger the buffer, the larger the maximum offset. This may appear only if the gstreamer fails to deliver frames for the other camera due to whatever reason.

As a frame pair is obtained, it will be passed to the DualFrameReceiver. This will be described in the next subsection.


2.2.2 Processing in threads

When gstreamer fires a callback function defined by class SimpleCapture, it is important that the callback function executes as fast as possible so as not to block gstreamer. Also, it is important to invoke SimpleCapture::grabFrame() as fast as possible so that frame drops will be avoided. To address the former constraint, SimpleCapture merely copies the frame from gstreamer and places it into its buffer, and additionally drops the oldest frame if the buffer is already full. The latter constraint can be achieved by placing the computationally demanding operations in their dedicated threads. These operations consist of:

1. Write frames to files
2. Decode and analyse eye camera frame
3. Decode scene frame, only if GUI is active
4. Place decoded frames to the GUIs queue, only if GUI is active

This way a call to DualframeReceiver::framesReceived() by class VideoSync executes fast.

The highest priority in the application is to write the frames to files. Writing MJPG frames to files is generally a very fast operation, parts of a millisecond. However, occasionally the writing takes up to a second, which prevents the writing to take place in the DualFrameReceiver. For this reason the writing will be performed in a separate thread as well, a class derived from DataWriter. In addition there are two threads, one for each stream, where the decoding and analysis will be performed.

A single decoding and analysis thread (GTWorker or JPEGWorker, both subclasses of class StreamWorker) must finish processing within 1 / fps seconds in order to keep up with the stream. However, the queue in StreamWorker allows for some flexibility with regards to this. Random processing times can exceed the maximum time, as long as this queue does not overflow. If overflow occurs, any new frame placed into the queue will be dropped. In order to have the StreamWorker outputs synchronised DualFrameReceiver will place the frames into StreamWorker queues only if both queues are not full.

After StreamWorker has finished processing the frame, it calls DualFrameReceiver::frameProcessed() which places the frame an output frame pair container. In case the container is already full the oldest pair will be removed. This container is accessible to the GUI.

NOTE:
In order to have the cameras stream raw frames instead of MJPG frames, opencv could be used. This modification needs to be done in TwoCameraTracker/gazetoworld/VideoSync.cpp. The original idea was to save the bus bandwidth by streaming MJPG. It turns out that the cameras output corrupt frames rarely, but when it happens, the jpeg converter calls exit() or something.


Threading is a very important method in contemporary applications, because most modern computers can divide the computational load to multiple processors. Also single-processor computers can show increase in computational power when threaded. See: Hyper-Threading http://en.wikipedia.org/wiki/Hyper-threading.


   camera 1                 camera 2
   ________________       _________________
   | SimpleCapture |      | SimpleCapture |
   -----------------      -----------------
             \              /
              \            /
            ___\__________/___
            |   VideoSync    |
            ------------------
                    |
                    | frame pair
          __________|__________
          | DualFrameReceiver |
          ---------------------
           |        |        |
   frame 1 |        |        | frame 2
   ________|___     |  ______|_______
   | GTWorker |     |  | JPEGWorker |
   ------------     |  --------------
           |  \     |         |
           |   \res |         |
           |  __\ __|______   |
           |  | DataWriter |  |
           |  --------------  |
            \                /
           __\______________/_
           |   GUI active    |
           -------------------
              |          |
        false |          | true
         __________   ____|____________
         | Delete |   | Collect pairs |
         ----------   | for the GUI   |
                      -----------------

