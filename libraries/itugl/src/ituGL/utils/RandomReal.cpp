#include "ituGL/utils/RandomReal.h"

std::random_device RandomReal::m_randomDevice;

RandomReal RandomReal::m_random;

RandomReal::RandomReal()
{
	m_engine = std::mt19937(m_randomDevice());
	std::uniform_real_distribution distribution(0.0f, 1.0f);
	this->m_distribution = distribution;
}

RandomReal::RandomReal(float min, float max)
{
	m_engine = std::mt19937(m_randomDevice());
	std::uniform_real_distribution distribution(min, max);
	this->m_distribution = distribution;
}

RandomReal::RandomReal(float min, float max, uint32_t seed)
{
	m_engine = std::mt19937(seed);
	std::uniform_real_distribution distribution(min, max);
	this->m_distribution = distribution;
}

void RandomReal::SetRange(float min, float max)
{
	m_distribution = std::uniform_real_distribution(min, max);
}

void RandomReal::NextSeed()
{
	m_engine = std::mt19937(m_randomDevice());
}

void RandomReal::SetSeed(uint32_t seed)
{
	m_engine.seed(seed);
}

float RandomReal::Get()
{
	return m_distribution(m_engine);
}

float RandomReal::Get(float min, float max)
{
	std::uniform_real_distribution distribution(min, max);
	return distribution(m_engine);
}

RandomReal& RandomReal::GetRandom()
{
	return m_random;
}
