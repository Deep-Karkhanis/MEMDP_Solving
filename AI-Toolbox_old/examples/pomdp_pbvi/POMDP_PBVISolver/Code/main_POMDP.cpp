/* ---------------------------------------------------------------------------
** Using POMDP from the AI-Toolbox 
**
** Author: Deep Karkhanis
** Email: deepkarkhanis@gmail.com
** -------------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <tuple>
#include <math.h>
#include <chrono>
// #include "utils.hpp"
#include <vector>
#include <string>

#include <AIToolbox/POMDP/IO.hpp>
#include <AIToolbox/POMDP/Algorithms/PBVI.hpp>
#include <AIToolbox/POMDP/Policies/Policy.hpp>

#include "model.hpp"
#include "recomodel.hpp"
#include "mazemodel.hpp"


std::string current_time_str();

/**
 * CURRENT_TIME_STR
 */
std::string current_time_str() {
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  time (&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
  return std::string(buffer);
}

template <typename M, typename P>
void evaluate_policy(M model, P policy, int n_sessions, int horizon, int session_len=400) {
	
	AIToolbox::POMDP::Belief belief;
  
	int subgroup_size = n_sessions / (int)(model.getE());
  	n_sessions = n_sessions - n_sessions % (int)(model.getE());
 	
  std::cout<<"Horizon is "<<horizon<<std::endl;

  for (int user = 0; user < n_sessions; user++) {
		
		  auto cluster = user%(int)(model.getE());
	    std::cerr << "\r     User " << user + 1 << "/" << n_sessions << std::string(15, ' ');
   		auto state = cluster * model.getO() + 0;

   		//build Belief
   		belief = AIToolbox::POMDP::Belief::Zero(model.getS());
   		for(int i=0 ; i<model.getE(); i++)
   		{
   			belief(i*model.getO() + 0) = 1.0/(model.getE());
   		}

   		std::cout<<std::endl<<std::endl<<"EVALUATION NUMBER "<<(user+1)<<" STARTED";
	    std::cout<<std::endl<<"Environment Chosen :"<<model.get_env(state)<<" ";
    	std::cout<<"Path followed: "<<model.state_to_string(state)<<" ";//<<prediction<<" ";

   		//get action
   		int chorizon = horizon,csession_len=session_len;
   		// auto act = policy.sampleAction(belief,chorizon);
      auto act = policy.sampleAction(belief,csession_len);
   		// --chorizon;
   		--csession_len;
   		
      for(int i=1;i<=session_len-1;i++)
   		{
   			auto next_tup = model.sampleSOR(state,std::get<0>(act));	
   			act=policy.sampleAction(std::get<1>(act),std::get<1>(next_tup),csession_len);
   			// chorizon--;
   			csession_len--;
   			state=std::get<0>(next_tup);

   			std::cout<<model.state_to_string(state)<<" ";
   			
   			if(model.isTerminal(state) || csession_len<=0)
   				break;
   		}
   		std::cout<<std::endl<<"Eval "<<(user+1)<<" Done"<<std::endl; 

   		
 	}
}

template <typename M>
void mainPOMDP(M model, std::string datafile_base,int nBeliefs, int steps, int horizon, float epsilon, bool precision,bool verbose) {
  // Solve Model
  auto start = std::chrono::high_resolution_clock::now();
  std::cout << "\n" << current_time_str() << " - Starting POMDP ValueIteration solver\n" << std::flush;
  AIToolbox::POMDP::PBVI solver(nBeliefs,horizon, epsilon);
  auto solution = solver(model);

  std::cout << current_time_str() << " - Convergence criterion e = " << epsilon << " reached ? " << std::boolalpha << std::get<0>(solution) << "\n" << std::flush;
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  double training_time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.;

  // Build and Evaluate Policy
  start = std::chrono::high_resolution_clock::now();
  std::cout << "\n" << current_time_str() << " - Starting evaluation!\n" << std::flush;
  AIToolbox::POMDP::Policy policy(model.getS(), model.getA(), model.getO(),std::get<1>(solution));
  std::cout << std::flush;
  std::cerr << std::flush;
  // evaluate_from_file(datafile_base + ".test", model, policy, 0, verbose);
    std::cout << "\nSaving policy to file for later usage...\n";
    {
        // You can load up this policy again using ifstreams.
        // You will not need to solve the model again ever, and you
        // can embed the policy into any application you want!
        std::ofstream output((datafile_base+"policy.txt"));
    }
  evaluate_policy(model,policy,1000,horizon);

  elapsed = std::chrono::high_resolution_clock::now() - start;
  double testing_time = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() / 1000000.;

  // Output Times
  std::cout << current_time_str() << " - Timings\n" << std::flush;
  std::cout << "   > Training : " << training_time << "s\n";
  std::cout << "   > Testing : " << testing_time << "s\n";
}


/**
 * MAIN ROUTINE
 */
int main(int argc, char* argv[]) {
  // Parse input arguments
  assert(("Usage: ./main file_basename data_mode [Discount] [nsteps] [precision]", argc >= 3));
  std::string data = argv[2];
  assert(("Unvalid data mode", !(data.compare("reco") && data.compare("maze"))));
  double discount = ((argc > 3) ? std::atof(argv[3]) : 0.95);
  assert(("Unvalid discount parameter", discount > 0 && discount < 1));
  int steps = ((argc > 4) ? std::atoi(argv[4]) : 1000000);
  assert(("Unvalid steps parameter", steps > 0));
  float epsilon = ((argc > 5) ? std::atof(argv[5]) : 0.01);
  assert(("Unvalid epsilon parameter", epsilon >= 0));
  bool precision = ((argc > 6) ? (atoi(argv[6]) == 1) : false);
  bool verbose = ((argc > 7) ? (atoi(argv[7]) == 1) : false);

  int nBeliefs = ((argc > 8) ? atoi(argv[8]) : 10);
  int horizon = ((argc > 9) ? atoi(argv[9]) : 100);

  // Create model
  std::string datafile_base = std::string(argv[1]);
  std::cout << "\n" << current_time_str() << " - Loading appropriate model\n";
  if (!data.compare("reco")) {
    Recomodel model (datafile_base + ".summary", discount, true);
    assert(("Model does not enable MDP mode", model.mdp_enabled()));
    model.load_rewards(datafile_base + ".rewards");
    model.load_transitions(datafile_base + ".transitions", precision, precision, datafile_base + ".profiles");
    mainPOMDP(model, datafile_base, nBeliefs, steps, horizon, epsilon, precision, verbose);
  } else if (!data.compare("maze")) {
    std::cout<<datafile_base<<std::endl;
    Mazemodel model(datafile_base + ".summary", discount);
    // assert(("Model does not enable MDP mode", model.mdp_enabled()));
    model.load_rewards(datafile_base + ".rewards");
    model.load_transitions(datafile_base + ".transitions", precision, precision);
    mainPOMDP(model, datafile_base, nBeliefs, steps, horizon, epsilon, precision, verbose);
  }
  return 0;
}
