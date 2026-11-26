#include"DeferedDeleter.h"
#include"../Log.h"

namespace RHI{
  DeferedDeleter::~DeferedDeleter()
  {
    LOG_INFO("defer deleter destory");
  }
  void DeferedDeleter::push(std::uint64_t waitValue,std::function<void()> deleter)
  {
    std::lock_guard<std::mutex> guard{m_mutex};
    m_deferedDeletions.push_back({waitValue,deleter});
  }
  void DeferedDeleter::deferedDeletion(VkDevice device,VkSemaphore semaphore)
  {
    std::uint64_t finishedValue{0};
    vkGetSemaphoreCounterValue(device, semaphore, &finishedValue);
    {
      std::lock_guard<std::mutex> guard{m_mutex};
      while(!m_deferedDeletions.empty() && m_deferedDeletions.front().wait <= finishedValue)
      {
        m_ready.push_back(std::move(m_deferedDeletions.front()));
        m_deferedDeletions.pop_front();
      }
    } 
    
    for(auto& i:m_ready)
    {
      i.deleter();
    }
    m_ready.clear();
  }
  void DeferedDeleter::flushAll()
  {
    for(auto& i:m_deferedDeletions)
    {
      i.deleter();
    }
    m_deferedDeletions.clear();
    m_ready.clear();
  }
}
