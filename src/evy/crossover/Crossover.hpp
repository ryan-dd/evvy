#ifndef EVY_CROSSOVER_HPP
#define EVY_CROSSOVER_HPP

#include "../rng/StdProbabilityRng.hpp"

#include <ranges>

namespace evy
{

template<
  std::invocable CrossoverProbability,
  typename CrossoverStrategy,
  std::invocable ProbabilityGenerator = StdProbabilityRng
>
class Crossover
{
public:
  CrossoverProbability crossoverProbability;
  CrossoverStrategy crossoverStrategy;
  ProbabilityGenerator generator = ProbabilityGenerator{};

  template<std::ranges::range ChromosomeContainer>
  requires std::invocable<CrossoverStrategy, 
           std::ranges::range_value_t<ChromosomeContainer>&, 
           std::ranges::range_value_t<ChromosomeContainer>&>
  constexpr void operator()(ChromosomeContainer& chromosomes)
  {
    for(const auto& chromosomePair : chromosomes | std::views::chunk(2))
    {
      // If "chromosomes" are passed in with an odd size, we can't do crossover on the last element 
      // Since it has no matching pair
      if(chromosomePair.size() != 2) 
      {
        continue;
      }

      if (generator() < crossoverProbability())
      {
        crossoverStrategy(chromosomePair[0], chromosomePair[1]);
      }
    }
  }
};

} // namespace evy

#endif
