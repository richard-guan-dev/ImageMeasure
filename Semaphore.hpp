#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

namespace ImageMeasure {
class Semaphore {
    unsigned int count_;
    boost::mutex mutex_;
    boost::condition_variable condition_;

  public:
    explicit Semaphore ( unsigned int initial ) : count_ ( initial ) {}

    void set ( unsigned int count ) {
        this->count_ = count;
    }

    void signal() {
        {
            boost::lock_guard<boost::mutex> lock ( mutex_ );
            ++count_;
        }
        condition_.notify_one();
    }

    void wait() {
        boost::unique_lock<boost::mutex> lock ( mutex_ );
        while ( count_ == 0 ) {
            condition_.wait ( lock );
        }
        --count_;
    }
};
}