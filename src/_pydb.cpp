#include<iostream>
#include<string.h>

#include<pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <functional>

#include "db.hpp"
#include "thread_pool.hpp" 

namespace py = pybind11;

std::vector<Reader *> reader; 
std::vector<Parser *> parser; 
std::vector<Decoder *> decoder;
ThreadPool *thread_pool;
uint8_t *prefetch_im;
float *prefetch_label;
uint8_t *im;
float *label;

void work(uint8_t *im, float *label, int thread_id) {
	reader[thread_id]->next();
	parser[thread_id]->work(reader[thread_id]->sample_str);
	decoder[thread_id]->work(parser[thread_id]->im_str, parser[thread_id]->label_str, im, label);
} 

void reset(py::dict param) {
	PyObject *ptr_as_int;
	std::string rec_path = py::cast<std::string>(param["rec_path"]);
	std::string idx_path = py::cast<std::string>(param["idx_path"]);
	int pool_size = 16;
	reader.resize(pool_size);
	parser.resize(pool_size);
	decoder.resize(pool_size);
	for (int i = 0; i < pool_size; i++) 
	{
		reader[i] = new Reader(rec_path, idx_path);
		parser[i] = new Parser();
		decoder[i] = new Decoder();
	}
	thread_pool = new ThreadPool(pool_size);
	prefetch_im = new uint8_t[64*224*224*3];
	im = new uint8_t[64*224*224*3];
	prefetch_label = new float[64];
	label = new float[64];
	
	for (int i = 0;i < 64;i++) 
		thread_pool->submit(std::bind(work, prefetch_im+i*224*224*3, prefetch_label+i*1, std::placeholders::_1));
	
	std::cout<<"========Reset fininshed==========\n";
}

inline void *get_ptr(py::object batch)
{
	PyObject* py_batch = batch.release().ptr();
	PyObject* ptr_batch = PyObject_GetAttr(py_batch, PyUnicode_FromString("value"));
	void *ptr_void_batch = PyLong_AsVoidPtr(ptr_batch);
	return ptr_void_batch;
}

void next(py::object batch_im, py::object batch_label)
{
	thread_pool->waitforwork();
	memcpy(im, prefetch_im, 64*224*224*3*sizeof(uint8_t));
	memcpy(label, prefetch_label, 64*sizeof(float));
	
	for (int i = 0;i < 64;i++) 
		thread_pool->submit(std::bind(work, prefetch_im+i*224*224*3, prefetch_label+i*1, std::placeholders::_1));
	
	void *batch_im_ptr = get_ptr(batch_im);
	//std::memcpy(batch_im_ptr, im, 64*224*224*3*sizeof(uint8_t));
	gpu_copy(batch_im_ptr, im, 64*224*224*3*sizeof(uint8_t));
	
	void *batch_label_ptr = get_ptr(batch_label);
	gpu_copy(batch_label_ptr, label, 64*sizeof(float));
}

PYBIND11_MODULE(_pydb, m) {
	m.doc() = "data loader";
	m.def("reset", &reset, "create a data loader", py::arg("param"));
	m.def("next", &next, "next batch");
}
