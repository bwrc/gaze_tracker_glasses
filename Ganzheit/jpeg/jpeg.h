#ifndef JPEG_H
#define JPEG_H


#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>



class JPEG_Compressor {

	public:

		JPEG_Compressor();

		bool compress(const unsigned char *data, int w, int h, int bpp);
		bool save(const std::string &fname);

		const JOCTET *getCompressedData(size_t &sz);

	private:

};


class JPEG_Decompressor {

	public:

		JPEG_Decompressor();
		~JPEG_Decompressor();

		bool decompress(const unsigned char *jpg_packed_data, size_t insize, unsigned char *decompr_data);

		bool save(const std::string &fname);

	private:

		int w;
		int h;
		int bpp;

};



#endif

