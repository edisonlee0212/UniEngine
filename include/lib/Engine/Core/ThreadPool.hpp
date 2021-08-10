#pragma once
namespace UniEngine
{
namespace detail
{
template <typename T> class ThreadQueue
{
  public:
    bool push(T const &value)
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        this->q.push(value);
        return true;
    }
    // deletes the retrieved element, do not use for non integral types
    bool pop(T &v)
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        if (this->q.empty())
            return false;
        v = this->q.front();
        this->q.pop();
        return true;
    }
    bool empty()
    {
        std::unique_lock<std::mutex> lock(this->mutex);
        return this->q.empty();
    }

  private:
    std::queue<T> q;
    std::mutex mutex;
};
} // namespace detail

class ThreadPool
{

  public:
    ThreadPool()
    {
        this->Init();
    }
    ThreadPool(int nThreads)
    {
        this->Init();
        this->Resize(nThreads);
    }

    // the destructor waits for all the functions in the queue to be finished
    ~ThreadPool()
    {
        this->FinishAll(true);
    }

    // get the number of running threads in the pool
    int Size() const
    {
        return static_cast<int>(this->m_threads.size());
    }

    // number of idle threads
    int IdleAmount() const
    {
        return this->m_waitingThreadAmount;
    }
    std::thread &GetThread(int i)
    {
        return *this->m_threads[i];
    }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
    // nThreads must be >= 0
    void Resize(int nThreads)
    {
        if (!this->m_isStop && !this->m_isDone)
        {
            int oldNThreads = static_cast<int>(this->m_threads.size());
            if (oldNThreads <= nThreads)
            { // if the number of threads is increased
                this->m_threads.resize(nThreads);
                this->m_flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i)
                {
                    this->m_flags[i] = std::make_shared<std::atomic<bool>>(false);
                    this->SetThread(i);
                }
            }
            else
            { // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i)
                {
                    *this->m_flags[i] = true; // this thread will finish
                    this->m_threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    this->m_threadPoolCondition.notify_all();
                }
                this->m_threads.resize(nThreads); // safe to delete because the threads are detached
                this->m_flags.resize(nThreads);   // safe to delete because the threads have copies of shared_ptr of the
                                                  // flags, not originals
            }
        }
    }

    // empty the queue
    void ClearQueue()
    {
        std::function<void(int id)> *_f;
        while (this->m_threadPool.pop(_f))
            delete _f; // empty the queue
    }

    // pops a functional wrapper to the original function
    std::function<void(int)> Pop()
    {
        std::function<void(int id)> *_f = nullptr;
        this->m_threadPool.pop(_f);
        std::unique_ptr<std::function<void(int id)>> func(
            _f); // at return, delete the function even if an exception occurred
        std::function<void(int)> f;
        if (_f)
            f = *_f;
        return f;
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the
    // functions
    void FinishAll(bool isWait = false)
    {
        if (!isWait)
        {
            if (this->m_isStop)
                return;
            this->m_isStop = true;
            for (int i = 0, n = this->Size(); i < n; ++i)
            {
                *this->m_flags[i] = true; // command the threads to stop
            }
            this->ClearQueue(); // empty the queue
        }
        else
        {
            if (this->m_isDone || this->m_isStop)
                return;
            this->m_isDone = true; // give the waiting threads a command to finish
        }
        {
            std::unique_lock<std::mutex> lock(this->m_mutex);
            this->m_threadPoolCondition.notify_all(); // stop all waiting threads
        }
        for (int i = 0; i < static_cast<int>(this->m_threads.size()); ++i)
        { // wait for the computing threads to finish
            if (this->m_threads[i]->joinable())
                this->m_threads[i]->join();
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the
        // threads therefore delete them here
        this->ClearQueue();
        this->m_threads.clear();
        this->m_flags.clear();
        this->m_waitingThreadAmount = 0;
        this->m_isStop = false;
        this->m_isDone = false;
    }

    template <typename F, typename... Rest> auto Push(F &&f, Rest &&...rest) -> std::future<decltype(f(0, rest...))>
    {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...));
        auto _f = new std::function<void(int id)>([pck](int id) { (*pck)(id); });
        this->m_threadPool.push(_f);
        std::unique_lock<std::mutex> lock(this->m_mutex);
        this->m_threadPoolCondition.notify_one();
        return pck->get_future();
    }

    // run the user's function that excepts argument int - id of the running thread. returned value is templatized
    // operator returns std::future, where the user can get the result and rethrow the catched exceptins
    template <typename F> auto Push(F &&f) -> std::future<decltype(f(0))>
    {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
        auto _f = new std::function<void(int id)>([pck](int id) { (*pck)(id); });
        this->m_threadPool.push(_f);
        std::unique_lock<std::mutex> lock(this->m_mutex);
        this->m_threadPoolCondition.notify_one();
        return pck->get_future();
    }

  private:
    // deleted
    ThreadPool(const ThreadPool &);            // = delete;
    ThreadPool(ThreadPool &&);                 // = delete;
    ThreadPool &operator=(const ThreadPool &); // = delete;
    ThreadPool &operator=(ThreadPool &&);      // = delete;

    void SetThread(int i)
    {
        std::shared_ptr<std::atomic<bool>> flag(this->m_flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag /* a copy of the shared ptr to the flag */]() {
            std::atomic<bool> &_flag = *flag;
            std::function<void(int id)> *_f;
            bool isPop = this->m_threadPool.pop(_f);
            while (true)
            {
                while (isPop)
                { // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(
                        _f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                        return; // the thread is wanted to stop, return even if the queue is not empty yet
                    else
                        isPop = this->m_threadPool.pop(_f);
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(this->m_mutex);
                ++this->m_waitingThreadAmount;
                this->m_threadPoolCondition.wait(lock, [this, &_f, &isPop, &_flag]() {
                    isPop = this->m_threadPool.pop(_f);
                    return isPop || this->m_isDone || _flag;
                });
                --this->m_waitingThreadAmount;
                if (!isPop)
                    return; // if the queue is empty and this->isDone == true or *flag then return
            }
        };
        this->m_threads[i].reset(new std::thread(f)); // compiler may not support std::make_unique()
    }

    void Init()
    {
        this->m_waitingThreadAmount = 0;
        this->m_isStop = false;
        this->m_isDone = false;
    }

    std::vector<std::unique_ptr<std::thread>> m_threads;
    std::vector<std::shared_ptr<std::atomic<bool>>> m_flags;
    detail::ThreadQueue<std::function<void(int id)> *> m_threadPool;
    std::atomic<bool> m_isDone;
    std::atomic<bool> m_isStop;
    std::atomic<int> m_waitingThreadAmount; // how many threads are waiting

    std::mutex m_mutex;
    std::condition_variable m_threadPoolCondition;
};

} // namespace UniEngine