#include "ResultParser.h"
#include <stdio.h>



bool ResultParser::parsePacket(const char *buff, const int len, ResultData &data) {

	// the string with whitespace removed, will contain the trailing '\0'
	char *str;

	// get rid of whitespace, adds the trailing '\0'
	ResultParser::removeWhitespace(buff, len, &str);

	// return value is true unless we counter an error
	bool retval = true;

	// a pointer to the string at the start of the field
	char *ptr = str;


	/*********************************************
	 * Loop while there are commas ',' in ptr
	 *********************************************/
	while(ptr != NULL) {

		// contains the extracted values
		std::list<double> values;

		/* 
		 * if ptr != str, then ptr is at a ','. In this case look at
		 * the next character, else look at ptr itself.
		 */
		char *c = ptr != str ? ptr + 1 : str;


		if(*c == '[') {

			/**************************************************
			 * Multiple values in this field, within "[...]"
			 **************************************************/

			/*
			 */
			char *ptr_start = c + 1;

			// try to find the matching brace ']'
			char *ptr_end_brace = strchr(ptr_start, ']');
			if(ptr_end_brace == NULL) {

				// the matching brace was not found
				retval = false;

				// break the loop
				break;

			}

			// parseField() requires the trailing '\0'
			*ptr_end_brace = '\0';

			parseField(ptr_start, values);
			*ptr_end_brace = ']';


			/*
			 * ptr must be set to point at the end brace ']' character
			 * because all possible ',' characters were already handled here.
			 */
			ptr = ptr_end_brace;

		}

		else {

			/**************************************************
			 * A single value field
			 **************************************************/

			char *ptr_start = *ptr != ',' ? ptr : ptr+1;
			char *ptr_end = strchr(ptr_start, ',');
			if(ptr_end != NULL) {
				*ptr_end = '\0';
			}
			parseField(ptr_start, values);
			if(ptr_end != NULL) {
				*ptr_end = ',';
			}
		}


		if(!data.insertNextData(values)) {
			delete[] str;
			return false;
		}

		// get the next token, ptr will be either at the next ',' or NULL
		ptr = strchr(ptr + 1, ',');

	}

	delete[] str;

	return retval;

}


/*
 * Must contain the trailing '\0'
 *	str : 10.00, 10.12, 89.54, 98.22\0
 */
void ResultParser::parseField(char *str, std::list<double> &values) {

	// look for ','
	char *ptr = strchr(str, ',');


	// if no ',' was found, the string is a single-element field
	if(ptr == NULL) {

		values.push_back(atof(str));

	}
	else {

		ptr = str;

		while(ptr != NULL) {

			// pointer at the next comma
			char *ptr_comma = strchr(ptr, ',');

			/*
			 * atof() -description
			 *
			 * "The function first discards as many whitespace characters as
			 * necessary until the first non-whitespace character is found.
			 * Then, starting from this character, takes as many characters
			 * as possible that are valid following a syntax resembling that
			 * of floating point literals, and interprets them as a numerical
			 * value. The rest of the string after the last valid character is
			 * ignored and has no effect on the behavior of this function."
			 *
			 */

			values.push_back(atof(ptr));

			ptr = ptr_comma != NULL ? ptr_comma + 1 : NULL;

		}

	}

}


////////void ResultParser::printData(const ResultData &data) {

////////	printf("ID                   : %ld\n"
////////		   "timestamp            : %ld\n"
////////		   "track duration (us)  : %ld\n"
////////		   "pupil ellipse        : %.2f, %.2f, %.2f, %.2f, %.2f\n"
////////		   "cornea centre        : %.2f, %.2f, %.2f\n"
////////		   "pupil centre         : %.2f, %.2f, %.2f\n"
////////		   "scene point          : %.2f, %.2f\n"
////////		   "glints               : ",
////////			(long)data.id,
////////			(long)data.timestamp,
////////			(long)data.track_dur_micros,
////////			data.pupil.center.x,
////////			data.pupil.center.y,
////////			data.pupil.size.width,
////////			data.pupil.size.height,
////////			data.pupil.angle,
////////			data.cornea_centre.x,
////////			data.cornea_centre.y,
////////			data.cornea_centre.z,
////////			data.pupil_centre.x,
////////			data.pupil_centre.y,
////////			data.pupil_centre.z,
////////			data.scenePoint.x,
////////			data.scenePoint.y);

////////	const std::vector<cv::Point2d> &crs = data.glints;
////////	std::vector<cv::Point2d>::const_iterator it = crs.begin();

////////	while(it != crs.end()) {

////////		printf("(%.2f, %.2f) ", it->x, it->y);

////////		++it;

////////	}

////////	printf("\n");

////////}


void ResultParser::removeWhitespace(const char *buff, const int len, char **_str) {

	// include the trailing '\0'
	*_str = new char[len + 1];
	char *str = *_str;


	char *ptr = str;

	for(int i = 0; i < len; ++i) {

		const char c = buff[i];

		if(c != ' ' && c != '\t' && c != '\r') {

			*ptr++ = c;

		}

	}

	int newlen = ptr - str;

	// insert the trailing '\0'
	str[newlen] = '\0';

}

