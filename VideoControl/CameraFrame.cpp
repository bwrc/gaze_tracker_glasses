#include "CameraFrame.h"
#include <string.h>


VideoInfo::VideoInfo() {
	w = h = fps = 0;

	// initially no save path is defined
	save_path = std::string("null");
}


CameraFrame::CameraFrame(int _w,
						 int _h,
						 int _bpp,
						 unsigned char *_data,
						 size_t _sz,
						 Format _format,
						 bool b_copy_data,
						 bool b_become_parent) {

	w			= _w;
	h			= _h;
	bpp			= _bpp;
	format		= _format;
	sz			= _sz;

	if(b_copy_data) {
		b_own_data	= true;
	}
	else {
		b_own_data	= b_become_parent;
	}

	if(w != 0 && h != 0 && bpp != 0 && sz != 0) {

		if(_data != NULL) {

			if(b_copy_data) {

				data = new unsigned char[sz];
				memcpy(data, _data, sz);

			} else {
				data = _data;
			}
		}

	}
	else {
		data = NULL;
	}

}


CameraFrame::CameraFrame(const CameraFrame &orig) {

	w		= orig.w;
	h		= orig.h;
	bpp		= orig.bpp;
	format	= orig.format;
	sz		= orig.sz;
	b_own_data = true; // set to true regardless of the orig's data

	if(sz != 0) {
		data = new unsigned char[sz];
		memcpy(data, orig.data, sz);
	}
	else {
		data = NULL;
	}

}


/* http://en.wikipedia.org/wiki/Assignment_operator_%28C%2B%2B%29 */
CameraFrame &CameraFrame::operator= (const CameraFrame & other) {

	// protect against invalid self-assignment
	if(this != &other) {

		// 1: allocate new memory and copy the elements
		unsigned char *new_data = new unsigned char[other.sz];

		memcpy(new_data, other.data, other.sz);

		// 2: deallocate old memory
		if(b_own_data) {
			delete[] data;
		}

		// 3: assign the new memory to the object
		data = new_data;

		w		= other.w;
		h		= other.h;
		bpp		= other.bpp;
		format	= other.format;
		sz		= other.sz;

		b_own_data = true; // set to true regardless of the other's data

	}

	// by convention, always return *this
	return *this;

}


CameraFrame::~CameraFrame() {

	if(b_own_data) {
		delete[] data;
	}

}


void CameraFrame::create(int _w, int _h, int _bpp, const unsigned char *_data, size_t _sz, Format _format) {

	size_t sz_old = sz;

	w = _w;
	h = _h;
	bpp = _bpp;
	format = _format;
	sz = _sz;

	// allocate only if sizes differ
	if(sz_old != sz) {
		if(b_own_data) {
			delete[] data;
		}
		data = new unsigned char[sz];
	}

}


void CameraFrame::release() {

	if(b_own_data) {
		delete[] data;
	}
	data = NULL;
	w = h = bpp = sz = 0;
	format = FORMAT_RGB;

}

