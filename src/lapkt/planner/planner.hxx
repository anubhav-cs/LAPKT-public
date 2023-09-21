#ifndef __PLANNER__
#define __PLANNER__
#include <lapkt/model/fluent.hxx>
#include <lapkt/model/strips_prob.hxx>
#include <lapkt/model/search_prob.hxx>

class Planner
{
public:
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
