#include <string.h>
#include "jpeg.h"
#include <string.h>


static const int BLOCK_SIZE = 16384;
std::vector<JOCTET> compressed_data;




/****************************************************************
 * These functions are callbacks used by the jpeg_destination_mgr
 ****************************************************************/


/****************************************************************
 * called when jpeg initialises itself
 ****************************************************************/
static void init_destination(j_compress_ptr cinfo) {

    compressed_data.resize(BLOCK_SIZE);
    cinfo->dest->next_output_byte = &compressed_data[0];
    cinfo->dest->free_in_buffer = compressed_data.size();

}


/****************************************************************
 * called when jpeg the buffer size needs to be increased
 ****************************************************************/
static boolean empty_output_buffer(j_compress_ptr cinfo) {

    size_t oldsize = compressed_data.size();
    compressed_data.resize(oldsize + BLOCK_SIZE);
    cinfo->dest->next_output_byte = &compressed_data[oldsize];
    cinfo->dest->free_in_buffer = compressed_data.size() - oldsize;

    return true;
}


/****************************************************************
 * called when jpeg has finished, and the buffer contains too
 * much data
 ****************************************************************/
static void term_destination(j_compress_ptr cinfo) {

    compressed_data.resize(compressed_data.size() - cinfo->dest->free_in_buffer);

}




JPEG_Compressor::JPEG_Compressor() {

}


bool JPEG_Compressor::compress(const unsigned char *data, int w, int h, int bpp) {

	// jpeg structures
	struct jpeg_compress_struct cinfo;
	struct jpeg_destination_mgr dmgr;
	struct jpeg_error_mgr jerr;

	/*********************************************************
	 * initialise the destination manager
	 *********************************************************/
	dmgr.init_destination		= &init_destination;
	dmgr.empty_output_buffer	= &empty_output_buffer;
	dmgr.term_destination		= &term_destination;


	/*********************************************************
	 * "update error manager with error handling routines"
	 *********************************************************/
	cinfo.err = jpeg_std_error(&jerr);


	/*********************************************************
	 * initialise the compressor
	 *********************************************************/
	jpeg_create_compress(&cinfo);


	// give the destination manager
	cinfo.dest = &dmgr;


	/*********************************************************
	 * set image data, dimensions etc.
	 *********************************************************/
	cinfo.image_width		= w;
    cinfo.image_height		= h;
    cinfo.input_components	= bpp;
    cinfo.in_color_space	= JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 120, true);


	/*********************************************************
	 * "The function jpeg_start_compress() shall initialize
	 * state for a JPEG compression cycle. The compression
	 * parameters, data destination and source image information
	 * must be set prior to the invocation of jpeg_start_compress().
	 * Setting write_all_tables to TRUE shall indicate that a
	 * complete JPEG interchange datastream will be written and
	 * all Huffman tables shall be emited. If write_all_tables is
	 * set to FALSE, the default behavior shall be to emit a pure
	 * abbreviated image with no tables."
	 *********************************************************/
	jpeg_start_compress(&cinfo, TRUE);


	int row_stride = w * bpp;
	JSAMPROW row_ptr[1];

	while(cinfo.next_scanline < cinfo.image_height) {
		row_ptr[0] = (JSAMPROW)&data[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_ptr, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);


	return true;

}


const JOCTET *JPEG_Compressor::getCompressedData(size_t &sz) {

	sz = compressed_data.size();
	return compressed_data.data();

}


bool JPEG_Compressor::save(const std::string &fname) {

	return true;
}





JPEG_Decompressor::JPEG_Decompressor() {

	w = h = bpp = 0;

}


JPEG_Decompressor::~JPEG_Decompressor() {

}












static void init_source(j_decompress_ptr cinfo) {

	/* already done in the JPEG_Decompressor::decompress() -method */

}


/*
 * In our case this method should not be called, because the
 * entire JPEG data is in the memory
 */
static boolean fill_input_buffer(j_decompress_ptr cinfo) {

	/*http://doxygen.reactos.org/d0/d1c/jdatasrc_8c_source.html*/

	static JOCTET mybuffer[4];

	/*
	 * The whole JPEG data is expected to reside in the supplied memory
	 * buffer, so any request for more data beyond the given buffer size
	 * is treated as an error.
	 */
//	WARNMS(cinfo, JWRN_JPEG_EOF);
	/* Insert a fake EOI marker */
	mybuffer[0] = (JOCTET) 0xFF;
	mybuffer[1] = (JOCTET) JPEG_EOI;

	cinfo->src->next_input_byte = mybuffer;
	cinfo->src->bytes_in_buffer = 2;

	return TRUE;

}


static void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {

	struct jpeg_source_mgr *src = cinfo->src;
	src->next_input_byte += (size_t) num_bytes;
	src->bytes_in_buffer -= (size_t) num_bytes;

}


static void term_source(j_decompress_ptr cinfo) {

}








/* JPEG DHT Segment for YCrCb omitted from MJPEG data */
static const
unsigned char jpeg_odml_dht[0x1a4] = {
	0xff, 0xc4, 0x01, 0xa2,

	0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

	0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

	0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa,

	0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa
};


/*
 * Parse the DHT table.
 * This code comes from jpeg6b (jdmarker.c).
  */
static
int qt_jpeg_load_dht (struct jpeg_decompress_struct *info, const unsigned char *dht,
	       JHUFF_TBL *ac_tables[], JHUFF_TBL *dc_tables[])
{
    unsigned int length = (dht[2] << 8) + dht[3] - 2;
    unsigned int pos = 4;
    unsigned int count, i;
    int index;

    JHUFF_TBL **hufftbl;
    unsigned char bits[17];
    unsigned char huffval[256];

    while (length > 16)
    {
	bits[0] = 0;
	index = dht[pos++];
	count = 0;
	for (i = 1; i <= 16; ++i)
	{
	    bits[i] = dht[pos++];
	    count += bits[i];
	}
	length -= 17;

	if (count > 256 || count > length)
	    return -1;

	for (i = 0; i < count; ++i)
	    huffval[i] = dht[pos++];
	length -= count;

	if (index & 0x10)
	{
	    index -= 0x10;
	    hufftbl = &ac_tables[index];
	}
	else
	    hufftbl = &dc_tables[index];

	if (index < 0 || index >= NUM_HUFF_TBLS)
	    return -1;

	if (*hufftbl == NULL)
	    *hufftbl = jpeg_alloc_huff_table ((j_common_ptr)info);
	if (*hufftbl == NULL)
	    return -1;

	memcpy ((*hufftbl)->bits, bits, sizeof (*hufftbl)->bits);
	memcpy ((*hufftbl)->huffval, huffval, sizeof (*hufftbl)->huffval);
    }

    if (length != 0)
	return -1;

    return 0;
}


bool JPEG_Decompressor::decompress(const unsigned char *jpg_packed_data, size_t insize, unsigned char *oput) {

	// jpeg structures
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_source_mgr smgr;


	/*********************************************************
	 * "update error manager with error handling routines"
	 *********************************************************/
	cinfo.err = jpeg_std_error(&jerr);


	/*********************************************************
	 * initialise the decompressor
	 *********************************************************/
	jpeg_create_decompress(&cinfo);


	/*********************************************************
	 * initialise the source manager
	 *********************************************************/
	smgr.init_source		= &init_source;
	smgr.term_source		= &term_source;
	smgr.resync_to_restart	= jpeg_resync_to_restart; /* use default method */
	smgr.skip_input_data	= &skip_input_data;
	smgr.fill_input_buffer	= &fill_input_buffer;
	smgr.bytes_in_buffer	= insize;
	smgr.next_input_byte	= (JOCTET *)jpg_packed_data;


	// assing the source manager
	cinfo.src = &smgr;

	

	/***************************************************
	 * read the jpeg header
	 ***************************************************/
	int ret = jpeg_read_header(&cinfo, TRUE);

	if(ret != JPEG_HEADER_OK) {
		printf("JPEG_Decompressor::decompress(): Could not read the header\n");
		jpeg_destroy_decompress(&cinfo);
		return false;
	}



	if (cinfo.ac_huff_tbl_ptrs[0] == NULL &&
		cinfo.ac_huff_tbl_ptrs[1] == NULL &&
		cinfo.dc_huff_tbl_ptrs[0] == NULL &&
		cinfo.dc_huff_tbl_ptrs[1] == NULL ) {
		qt_jpeg_load_dht( &cinfo, jpeg_odml_dht, cinfo.ac_huff_tbl_ptrs,
			    cinfo.dc_huff_tbl_ptrs );
	}


	/*************************************************************************
	 * "The function jpeg_start_decompress() shall initialize state
	 * for a JPEG decompression cycle and allocate working memory.
	 * The JPEG datastream header must be read prior to the invokation
	 * of jpeg_start_decompress() to obtain the parameters for decompression."
	 *************************************************************************/
    if(!jpeg_start_decompress(&cinfo)) {
		printf("JPEG_Decompressor::decompress(): Could not start decompressing\n");
		jpeg_destroy_decompress(&cinfo);
		return false;
	}


	int row_stride = cinfo.output_width * cinfo.output_components;

	// jpeg_finish_decompress() deallocates this memory
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
	((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


	/*******************************************************
	 * set class data
	 *******************************************************/
	bpp = cinfo.output_components;
	w = cinfo.output_width;
	h = cinfo.output_height;


	while(cinfo.output_scanline < cinfo.output_height) {

		int row = cinfo.output_scanline;
		jpeg_read_scanlines(&cinfo, buffer, 1);

		memcpy(oput + bpp*row*w, buffer[0], bpp*w);

	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return true;

}




//////const unsigned char *JPEG_Decompressor::getRawData(size_t &sz) {

//////	sz = w*h*bpp;

//////	return decompressed_data;

//////}


bool JPEG_Decompressor::save(const std::string &fname) {

	return true;

}

