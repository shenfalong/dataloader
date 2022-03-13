#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <iostream>
#include <vector>
#include <queue>
  
#include <thread>
#include <mutex>
#include <condition_variable>

#include <functional>

class ThreadPool {
 public:
 	ThreadPool(int pool_size)
 	{
 		stop_ = false;
 		work_complete_ = true;
 		work_threads_.resize(pool_size);
 		
 		for (int i = 0;i < work_threads_.size();i++)
 			work_threads_[i] = std::thread(&ThreadPool::dowork, this, i);
 	}
 	
 	void submit(std::function<void(int)> work) 
 	{
 		//std::unique_lock will automatically unlock out of the scope, 
 		//that is why sometimes use {} to enclose some code.
    mtx_.lock();
    job_queue_.push(std::move(work));
    work_complete_ = false;
    mtx_.unlock(); 
    cv_.notify_one();
  }
  
	void dowork(int thread_id)
 	{	
 		while (true) 
 		{
 			std::function<void(int)> work;
	 		std::unique_lock<std::mutex> lock(mtx_);
	 		
	 	  while (job_queue_.empty() && !stop_)
	 	  	cv_.wait(lock);
	 	  
	 	  if (stop_)
	 	  	return;
	 	  
	 		work = std::move(job_queue_.front());
			job_queue_.pop();
			lock.unlock();
			
		  work(thread_id);
		 	
		 	lock.lock();
		  if (job_queue_.empty()) 
		  {
		  	work_complete_ = true;
		  	completed_.notify_one();
		  }
		  lock.unlock();
	  }
 	}
 	
 	void waitforwork() 
 	{
 		std::unique_lock<std::mutex> lock(mtx_);
 		while (!work_complete_)
 			completed_.wait(lock);
 	}
 	
 	~ThreadPool() 
 	{
 		mtx_.lock();
 		stop_ = true;
 		mtx_.unlock();
 		cv_.notify_all();
		for (int i = 0;i < work_threads_.size();i++)
			work_threads_[i].join();
  }
 private:
 	
 	std::vector<std::thread> work_threads_;
  std::queue<std::function<void(int)>> job_queue_;
  std::thread thread_;
  std::mutex mtx_;
  std::condition_variable cv_, completed_;
  
  bool stop_, work_complete_;
};
#endif// THREAD_POOL_HPP
