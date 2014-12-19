#ifndef RESULTPARSER_H
#define RESULTPARSER_H


#include <ctime>
#include <list>
#include "ResultData.h"


/*
 * The data is in ASCII format. The data must be represented followingly:
 *
 *    ID,time_stamp,track_duration_micorseconds,[x,y,w,h,ang],[x,y,z],[x,y,z],[x,y],[x,y],[x,y],...,[x,y]
 *
 * The order is:
 *   ID                   : id of this data
 *   time stamp           : time stamp of this data
 *   track duration (us)  : track duration in microseconds
 *   pupil ellipse        : ellipse representing the estimated pupil
 *   cornea centre 3D     : centre of the cornea in the eye camera coordinate system
 *   pupil centre 3D      : centre of the pupil in the eye camera coordinate system
 *   scene point          : the mapped point in the scene image
 *   glint centres        : glint centres
 *
 */




/*
 * Static functions for parsing the incoming data. Inserts the results into
 * the given data container.
 */
class ResultParser {

	public:

		static bool parsePacket(const char *buff, const int len, ResultData &data);

		static void removeWhitespace(const char *buff, const int len, char **str);

		static void parseField(char *str, std::list<double> &values);

	private:

		/*
		 * Possible data this class contains. Used to determine which data
		 * is being inserted currently in the insertNextData() function
		 */
		enum DATATYPE {ID					= 0,
					   TIMESTAMP			= 1,
					   TRACK_DUR_MICROS		= 2,
					   PUPIL_ELLIPSE		= 3,
					   CORNEA_CENTRE		= 4,
					   PUPIL_CENTRE			= 5,
					   SCENE_POINT			= 6,
					   GLINTS				= 7};

		/*
		 *	which data to insert:
		 *	  0: id
		 *	  1: timestamp
		 *	  2: track duration in microseconds
		 *	  3: pupil ellipse
		 *	  4: cornea centre
		 *	  5: pupil centre
		 *	  6: mapped scene image point
		 *	  7: glints
		 */
		DATATYPE data_type;

};



#endif

