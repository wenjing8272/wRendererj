#include<deque>
#include<mutex>
#include<functional>
#include<cstdint>
#include<vector>

#include<vulkan/vulkan.h>

namespace RHI{
  class DeferedDeleter
  {
  public:
    DeferedDeleter() = default;
    ~DeferedDeleter();
    DeferedDeleter(const DeferedDeleter& otherDeleter) = delete;
    DeferedDeleter operator=(const DeferedDeleter& otherDeleter) = delete;
    
    //Add the appropriate resource release timing and lambda deleters to the delayed deletion queue
    void push(std::uint64_t waitValue,std::function<void()> deleter);
    
    void deferedDeletion(VkDevice device,VkSemaphore semaphore);

    void flushAll();
  private:
    struct Entry{
      std::uint64_t wait{0};
      std::function<void()>deleter;
    };
    std::deque<Entry> m_deferedDeletions;
    std::mutex m_mutex{};
    std::vector<Entry> m_ready;
  };
}
