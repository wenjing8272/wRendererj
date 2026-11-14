#include"Pipeline.h"

namespace wrj {
	Pipeline::~Pipeline()
	{

	}
	void Pipeline::init(VkDevice device)
	{
		m_device = device;
	}
}