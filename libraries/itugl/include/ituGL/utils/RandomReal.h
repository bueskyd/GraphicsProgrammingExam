#pragma once

#include <random>

class RandomReal
{
private:
    std::mt19937 m_engine;
    std::uniform_real_distribution<float> m_distribution;
    static std::random_device m_randomDevice;
    static RandomReal m_random;
public:
    RandomReal();
    RandomReal(float min, float max);
    RandomReal(float min, float max, uint32_t seed);
    void NextSeed();
    void SetSeed(uint32_t seed);
    void SetRange(float min, float max);
    [[nodiscard]] float Get();
    [[nodiscard]] float Get(float min, float max);
    [[nodiscard]] static RandomReal& GetRandom();
};
