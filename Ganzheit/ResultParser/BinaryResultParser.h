#ifndef BINARY_RESULT_PARSER_H
#define BINARY_RESULT_PARSER_H


#include "ResultParser.h"


/*
 * The data is in binary format. The data must be represented followingly:
 *
 *    N,success,ID,time_stamp,track_duration_micorseconds,[x,y,w,h,ang],[x,y,z],[x,y,z],[x,y],[x,y],[x,y],...,[x,y]
 *
 * All members are represented in little-endian convention
 *   ----------------------|-----------------------|-----------------------
 *   | Parameter           | Description           | Number of bytes      |
 *   **********************************************************************
 *   | N bytes             | The size of this data | 4 bytes              |
 *   |                     | including this byte   |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | success             | Tracking success      | 1 byte               |
 *   |                     |                       |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | ID                  | id of this data       | 4 bytes              |
 *   ----------------------|-----------------------|-----------------------
 *   | Time stamp          | time stamp of         | 4 bytes              |
 *   |                     | this data             |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Track duration (us) | track duration        | 4 bytes              |
 *   |                     | in microseconds       |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Pupil ellipse       | ellipse representing  | 5 * 4 bytes          |
 *   |                     | the estimated pupil   |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Cornea centre 3D    | centre of the cornea  | 3 * 4 bytes          |
 *   |                     | in the eye camera     |                      |
 *   |                     | coordinate system     |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Pupil centre 3D     | centre of the pupil   | 3 * 4 bytes          |
 *   |                     | in the eye camera     |                      |
 *   |                     | coordinate system     |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Scene point         | the mapped point in   | 2 * 4 bytes          |
 *   |                     | the scene image       |                      |
 *   ----------------------|-----------------------|-----------------------
 *   | Number of glints    | Number of glints      | 2 bytes              |
 *   |                     |                       |                      |
 *   ----------------------------------------------------------------------
 *   | Glint centres       | glint centres         | 2 * n * 4 bytes      |
 *   |                     |                       | n = number of glints |
 *   ----------------------------------------------------------------------
 *   | N contours          | number of contours    | 2 bytes              |
 *   |                     |                       |                      |
 *   ----------------------------------------------------------------------
 *   | Contours            | list of contours      |                      |
 *   |                     |                       |                      |
 *   ----------------------------------------------------------------------
 *   | gaze vector start   | start image point     | 2*2 bytes            |
 *   |                     |                       |                      |
 *   ----------------------------------------------------------------------
 *   | gaze vector end     | end image point       | 2*2 bytes            |
 *   |                     |                       |                      |
 *   ----------------------------------------------------------------------
 *
 * The total number of bytes is therefore:
 *     4 + 1 + 4 + 4 + 4 + 20 + 12 + 12 + 8 + 2 + 8*nGlints + 2 + 4*nContours + 8*totalContourPoints + 4 + 4
 *     = 81 + 8*nGlints + 4*nContours + 4*totalContourPoints
 *
 */

class BinaryResultParser {

	public:

		static bool parsePacket(const char *buff, const int len, ResultData &data);

		static void resDataToBuffer(const ResultData &data, std::vector<char> &buff);

		enum LIMITS {

			MIN_BYTES = 81

		};

};


#endif

