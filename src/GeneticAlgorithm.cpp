#include <algorithm>
#include <cstddef>
#include <iostream>
#include <list>
#include <random>

#include "GeneticAlgorithm.hpp"

EvoAlgos::GeneticAlgorithm::GeneticAlgorithm(int pop_number, int max_iterations, int k_tournament_selection, double crossover_probability, double mutation_probability)
{
    this->_max_iterations = max_iterations;
    this->_pop_number = pop_number;
    this->_k_tournament_selection = k_tournament_selection;
    this->_crossover_probability = crossover_probability;
    this->_mutation_probability = mutation_probability;
    _scores = std::vector<double> (pop_number, 0);
}
    
EvoAlgos::GeneticAlgorithm::~GeneticAlgorithm()
{
}

std::vector<double> EvoAlgos::GeneticAlgorithm::run(EvoAlgos::OptimizationProblem problem)
{
    _current_population = generate_initial_solutions(problem);
    for (int i = 0; i < _max_iterations; ++i)
    {
        evaluate(_current_population, problem);
        std::vector<std::vector<double> > new_parents = select_parents();
        std::vector<std::vector<double> > new_population = crossover(new_parents);
        _current_population = mutate(new_population, problem);
    }

    return _get_best_chromosome();
}

std::vector<std::vector<double> > EvoAlgos::GeneticAlgorithm::generate_initial_solutions(EvoAlgos::OptimizationProblem problem)
{
    const int num_parameters = problem.get_num_parameters();
    const auto& constraints = problem.get_constraints();
    std::vector<std::vector<double> > initial_solutions(_pop_number, std::vector<double>(num_parameters, 0));
    std::mt19937 number_generator(std::random_device{}());
    for (int parameter_index = 0; parameter_index < num_parameters; ++parameter_index) {
        for (int population_index = 0; population_index < _pop_number; ++population_index) {
            std::uniform_real_distribution<> dist(constraints[parameter_index][0], constraints[parameter_index][1]);
            initial_solutions[population_index][parameter_index] = dist(number_generator);
        }
    }
    return initial_solutions;
}

void EvoAlgos::GeneticAlgorithm::evaluate(std::vector<std::vector<double> > solution_population, EvoAlgos::OptimizationProblem problem)
{
    for (std::size_t i = 0; i < solution_population.size(); ++i)
    {
        _scores[i] = problem.objective_function(solution_population[i]);
    }
}

std::vector<std::vector<double> > EvoAlgos::GeneticAlgorithm::select_parents()
{  
    std::vector<std::vector<double> > parents = std::vector<std::vector<double> >(_pop_number);
    for (std::size_t i = 0; i < parents.size(); ++i)
    {
        std::vector<int> run_indices = _pick_random_chromosome(_k_tournament_selection);
        int parent_index = _tournament_selection(run_indices);
        parents[i] = _current_population[parent_index];
    }
    return parents;
}

std::vector<int> EvoAlgos::GeneticAlgorithm::_pick_random_chromosome(int k)
{
    std::vector<int> all_indices(_pop_number);
    for (std::size_t i = 0; i < all_indices.size(); ++i)
        all_indices[i] = i;
    
    std::random_device rd;
    std::mt19937 prng(rd());
    std::shuffle(all_indices.begin(), all_indices.end(), prng);
    auto first = all_indices.cbegin();
	auto last = all_indices.cbegin() + k + 1;

    std::vector<int> random_indices(first, last);
	return random_indices;
}

int EvoAlgos::GeneticAlgorithm::_tournament_selection(std::vector<int> chromosome_indices)
{
    std::vector<double> selected_scores = std::vector<double>(chromosome_indices.size());
    for (std::size_t i = 0; i < selected_scores.size(); ++i)
    {
        selected_scores[i] = _scores[chromosome_indices[i]];
    }
    auto it = std::max_element(selected_scores.begin(), selected_scores.end());
    int max_index = it - selected_scores.begin();
    return chromosome_indices[max_index];
}

std::vector<std::vector<double> > EvoAlgos::GeneticAlgorithm::crossover(std::vector<std::vector<double> > population)
{
    std::mt19937 probability_generator(std::random_device{}());
    std::uniform_real_distribution<> real_dist(0, 1);

    std::mt19937 number_generator(std::random_device{}());
    std::uniform_int_distribution<> int_dist(0, population[0].size()-1);

    for (std::size_t i = 0; i < population.size()/2; ++i)
    {
        double probability = real_dist(probability_generator);
        if (probability < _crossover_probability)
        {
            int crossover_index = int_dist(number_generator);
            std::vector<double>& first = population[2*i];
            std::vector<double>& second = population[2*i+1];
            std::swap_ranges(first.begin(), first.begin()+crossover_index+1, second.begin());
        }

    }
    return population;
}

std::vector<std::vector<double> > EvoAlgos::GeneticAlgorithm::mutate(std::vector<std::vector<double> > population, EvoAlgos::OptimizationProblem problem)
{
    std::mt19937 probability_generator(std::random_device{}());
    std::uniform_real_distribution<> real_dist(0, 1);

    std::mt19937 number_generator(std::random_device{}());
    std::uniform_int_distribution<> int_dist(0, population[0].size()-1);

    std::mt19937 mutation_generator(std::random_device{}());

    for (std::size_t i = 0; i < population.size(); ++i)
    {
        double probability = real_dist(probability_generator);
        if (probability < _mutation_probability)
        {
            std::vector<double>& chromosome = population[i];
            int mutation_index = int_dist(number_generator);
            double number = real_dist(mutation_generator);
            double lower_bound = problem.get_constraints()[mutation_index][0];
            double upper_bound = problem.get_constraints()[mutation_index][1];
            chromosome[mutation_index] = number*(upper_bound-lower_bound)+lower_bound;
        }
    }
    return population;
}

std::vector<double> EvoAlgos::GeneticAlgorithm::_get_best_chromosome()
{
    auto it = std::max_element(_scores.begin(), _scores.end());
    int max_index = it - _scores.begin();
    return _current_population[max_index];
}
