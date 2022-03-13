#ifndef DB_HPP
#define DB_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <string.h> 
#include <tuple>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

void gpu_copy(void *a, void *b, size_t count);

struct ImageRecordIOHeader 
{
  uint32_t flag;
  float label;
  uint64_t image_id[2];
};

class Reader {
 public:
	Reader(std::string &rec_path, std::string &idx_path) 
	{
		in_ = new std::ifstream(rec_path, std::ifstream::in|std::ifstream::binary);
		std::ifstream idx_file(idx_path);
		std::vector<size_t> temp, temp_size, temp_rec;
    size_t offset, size, label;
    while (idx_file >> offset >> size >> label) 
    	indices_.push_back(std::make_tuple(offset, size));
    idx_file.close();
    current_index_ = 0;
	}
	void next() 
	{
		int64_t offset, size;
    std::tie(offset, size) = indices_[current_index_];
		current_index_++;
		
		in_->seekg(offset, in_->beg);
		
		sample_str.resize(size);
		in_->read(&sample_str[0], size);
	}
	~Reader() 
	{
		in_->close();
	}
	
	std::vector<char> sample_str;
 private:
	std::ifstream* in_;
	std::vector<std::tuple<int64_t, int64_t>> indices_;
	int current_index_;
	
};

class Parser 
{
 public:
	Parser() 
	{
	}
	void work(std::vector<char> &input_sample_str) 
	{
		char *sample_str = &input_sample_str[0];
		
		uint32_t magic;
		const uint32_t kMagic = 0xced7230a;
		memcpy(&magic, sample_str, sizeof(uint32_t));
		sample_str += sizeof(uint32_t);
		assert(magic == kMagic);

		uint32_t length_flag;
		memcpy(&length_flag, sample_str, sizeof(length_flag));
		sample_str += sizeof(uint32_t);

		uint32_t cflag = (length_flag >> 29U) & 7U;
		uint32_t clength = length_flag & ((1U << 29U) - 1U);

		ImageRecordIOHeader hdr;
		memcpy(&hdr, sample_str, sizeof(ImageRecordIOHeader));
		sample_str += sizeof(ImageRecordIOHeader);;

		int64_t data_size = clength - sizeof(ImageRecordIOHeader);
		int64_t label_size = hdr.flag * sizeof(float);
		int64_t image_size = data_size - label_size;
		if (label_size > 0)
		{
			label_str.resize(label_size);
			label_str.assign(sample_str, sample_str+label_size*sizeof(float));
			sample_str += label_size;
		}
		else
		{
			label_str.resize(1);
			label_str[0] = hdr.label;
		}
		im_str.resize(image_size);
		im_str.assign(sample_str, sample_str+image_size*sizeof(char));
		
		sample_str += image_size;
  }
	std::vector<char> im_str;
	std::vector<float> label_str;
 private:
	
};

class Decoder 
{
 public:
	Decoder() 
	{
	}
	void work(std::vector<char> &im_str, std::vector<float> &label_str, uint8_t *&im, float *&label) 
	{
		cv::Mat decode_im = cv::imdecode(im_str, cv::IMREAD_COLOR);
		cv::resize(decode_im, decode_im, cv::Size(224,224), 0, 0, cv::INTER_LINEAR);

		assert(decode_im.isContinuous());
    memcpy(im, decode_im.data, 224*224*3*sizeof(uint8_t));
    memcpy(label, &label_str[0], label_str.size()*sizeof(float));
  }
 private:
};
#endif //DB_HPP
