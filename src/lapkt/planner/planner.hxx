#ifndef __PLANNER__
#define __PLANNER__
#include <lapkt/model/fluent.hxx>
#include <lapkt/model/strips_prob.hxx>
#include <lapkt/model/search_prob.hxx>

class Planner
{
public:

  struct RunStatistics {
    unsigned num_fluents = 0;
    unsigned num_actions = 0;
    unsigned num_node_expanded = 0;
    unsigned num_node_generated = 0;
    unsigned highest_novelty = 0; // of nodes in "explicit search tree"
    // "explicit search tree" means the tree generated during the search

    float runtime = 0.0;
    float plan_cost = 0.0;
  };


  Planner(){}

  virtual ~Planner(){};

  virtual void setup(){};
  virtual void solve(){};
  virtual void solve(aptk::STRIPS_Problem* prob){};
  
  void print_action(int index);
  void print_fluents();
  void print_actions();

  aptk::STRIPS_Problem* instance()
  {
    return m_problem;
  }
  std::string get_atom_name(int idx) const
  {
    return m_problem->fluents()[idx]->signature();
  }
  std::string get_domain_name() const { return m_problem->domain_name(); }
  std::string get_problem_name() const { return m_problem->problem_name(); }

  std::vector<aptk::Action_Idx> plan;
  std::string m_log_filename;
  std::string m_plan_filename;
  RunStatistics statistics;

protected:
  template <typename Search_Engine>
  float do_search(Search_Engine &engine,
          aptk::STRIPS_Problem &plan_prob,
          std::ofstream &plan_stream);
  template <typename Search_Engine>
  float do_search_single_goal(
    Search_Engine &engine, aptk::STRIPS_Problem &plan_prob,
    std::ofstream &plan_stream);

  aptk::STRIPS_Problem *m_problem=nullptr;
};
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx//
#endif
