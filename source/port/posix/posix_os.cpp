#include "spn_os.h"
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <semaphore>
#include <thread>
#include <vector>

namespace spn_posix {

struct mailbox {
    const char name[32];
    size_t size;
    spn_os_priority_t prio;
    std::mutex mtx;
    std::vector<uintptr_t> queue;

    mailbox(const char* _name, size_t size, spn_os_priority_t prio)
        : name("")
        , size(size)
        , prio(prio)
    {
        std::strcpy((char*)name, _name);
    }

    bool tx(uintptr_t msg)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.size() >= size) {
            return false;
        }
        queue.push_back(msg);
        return true;
    }

    bool rx(uintptr_t* msg, uint32_t timeout)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty()) {
            if (timeout == 0) {
                return false;
            }
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(timeout));
            lock.lock();
            if (queue.empty()) {
                return false;
            }
        }
        *msg = queue.front();
        queue.erase(queue.begin());
        return true;
    }
};

struct timer {
    const char name[32];
    size_t size;
    spn_os_priority_t prio;
    uint32_t period;
    spn_os_timer_type_t type;
    void (*callback)(uintptr_t);
    uintptr_t arg;
    std::shared_ptr<std::thread> th;
    bool paused;

    timer(const char* _name, spn_os_priority_t prio, uint32_t period, spn_os_timer_type_t type, void (*callback)(uintptr_t), uintptr_t arg)
        : name("")
        , prio(prio)
        , period(period)
        , type(type)
        , callback(callback)
        , arg(arg)
        , paused(true)
    {
        std::strcpy((char*)name, _name);
    }

    void run()
    {
        while (true) {
            std::this_thread::sleep_for(std::chrono::microseconds(period));
            callback(arg);
            if (type == SPN_OS_TIMER_ONESHOT) {
                break;
            }
        }
    }

    void start()
    {
        th = std::make_shared<std::thread>(&timer::run, this);
    }

    void cancel()
    {
        // NOTE: This is a blocking call
        th->join();
    }
};

}

extern "C" {

spn_os_mailbox_t spn_mailbox_create(const char* name, size_t size, spn_os_priority_t prio)
{
    return (spn_os_mailbox_t)(new spn_posix::mailbox(name, size, prio));
}

bool spn_mailbox_tx(spn_os_mailbox_t mailbox, uintptr_t msg)
{
    return ((spn_posix::mailbox*)mailbox)->tx(msg);
}

bool spn_mailbox_rx(spn_os_mailbox_t mailbox, uintptr_t* msg, uint32_t timeout)
{
    return ((spn_posix::mailbox*)mailbox)->rx(msg, timeout);
}

void spn_mailbox_destroy(spn_os_mailbox_t mailbox)
{
    delete (spn_posix::mailbox*)mailbox;
}

spn_os_timer_t spn_timer_create(const char* name, spn_os_priority_t prio, uint32_t period, spn_os_timer_type_t type, void (*callback)(uintptr_t), uintptr_t arg)
{
    return (spn_os_timer_t)(new spn_posix::timer(name, prio, period, type, callback, arg));
}

void spn_timer_start(spn_os_timer_t timer)
{
    ((spn_posix::timer*)timer)->start();
}

void spn_timer_cancel(spn_os_timer_t timer)
{
    ((spn_posix::timer*)timer)->cancel();
}

void spn_timer_destroy(spn_os_timer_t timer)
{
    delete (spn_posix::timer*)timer;
}

void spn_yield()
{
    std::this_thread::yield();
}

void spn_msleep(uint32_t time)
{
    std::this_thread::sleep_for(std::chrono::microseconds(time));
}

void spn_get_time(spn_os_time_t* time)
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
    time->sec = value.count() / 1000;
    time->nano_sec = (value.count() % 1000) * 1000000;
}
}
