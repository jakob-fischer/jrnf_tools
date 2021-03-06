/* author: jakob fischer (mail@jakobfischer.eu)
 * date: 9th February 2013
 * description: 
 * Programm with C++ Tools for generating different types of complex
 * networks / reaction networks and transforming between them.
 * 
 * TODO: Complex network generation of 2-2 networks should have virtual links 
 *       included in connectivity check
 */

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <algorithm>
#include <boost/concept_check.hpp>

#include "net_tools/reaction_network.h"
#include "net_tools/reaction_network_fileop.h"
#include "net_tools/network_tools.h"
#include "tools/cl_para.h"
using namespace std;


/*
 *  Makro for diffusion connection of two species. (A -> B) reactions 
 * in both directions with all constants set to 1.0. 
 *  (The species 'a' and 'b' are connected by adding the respective
 *   reactions to the reaction vector re.)
 */

void rm_diffusion(vector<reaction>& re, size_t a, size_t b) {
    reaction rea_1;
	reaction rea_2;
	
	rea_1.add_educt(a);
	rea_1.add_product(b);
	
	rea_2.add_educt(b);
	rea_2.add_product(a);
	
	rea_1.set_c(1.0);
	rea_2.set_c(1.0);
	rea_1.set_k(1.0);
	rea_2.set_k(1.0);
	rea_1.set_k_b(1.0);
	rea_2.set_k_b(1.0);
	
	re.push_back(rea_1);
	re.push_back(rea_2);
}


/*
 * Macro for adding a reaction in the form "A ---> B". 
 * ae - activation energy
 */

void rm_1to1(vector<reaction>& re, size_t a, size_t b, double ae=0.0) {
    reaction rea;
    rea.set_activation(ae);
    rea.set_reversible(false);
    rea.add_educt(a);
    rea.add_product(b);
    
    re.push_back(rea);  
}


/*
 * Macro for adding a reaction in the form "A <--> B". 
 * ae - activation energy
 */

void rm_1to1rev(vector<reaction>& re, size_t a, size_t b, double ae=0.0) {
    reaction rea;
    rea.set_activation(ae);
    rea.set_reversible(true);
    rea.add_educt(a);
    rea.add_product(b);
    
    re.push_back(rea);  
}


/*
 * Macro for adding a reaction in the form "A + B ---> C + D". 
 * ae - activation energy
 */
 
void rm_2to2(vector<reaction>& re, size_t a, size_t b, size_t c, size_t d, double ae=0.0) {
    reaction rea;
    rea.set_activation(ae);
    rea.set_reversible(false);
    rea.add_educt(a);
    rea.add_educt(b);
    rea.add_product(c);
    rea.add_product(d);
    
    re.push_back(rea);  
}


/*
 * Macro for adding a reaction in the form "A + B <--> C + D". 
 * ae - activation energy
 * 
 * TODO unify if set_activation should get relative or absolute energy
 */

void rm_2to2rev(vector<reaction>& re, size_t a, size_t b, size_t c, size_t d, size_t ae_dist=0) {
    reaction rea;
    rea.set_reversible(true);
    rea.add_educt(a);
    rea.add_educt(b);
    rea.add_product(c);
    rea.add_product(d);
    
    if(ae_dist == 0) {
        rea.set_activation(double(rand())/RAND_MAX);
    } else {
        // WARNING Not implemented yet
	rea.set_activation(double(rand())/RAND_MAX);
    }
    
    re.push_back(rea);  
}


/*
 * Macro for adding a species to the vector `sp`.  Species is named 
 * "A_<t>", the energy distribution is set by energy_dist
 * 
 * TODO implement different energy distributions
 */

void rm_add_species(std::vector<species>& sp, size_t t, size_t energy_dist) {
    stringstream ss;
    ss << "A_" << t;
				
    sp.push_back(species(sp.size(), ss.str(), false, 0));
    if(energy_dist == 0) {
        sp.back().set_energy(-double(rand())/RAND_MAX);
    } else {
        // WARNING Not implemented yet
	    sp.back().set_energy(double(rand())/RAND_MAX);
    }
}


/*
 * Macro for adding a species to the vector `sp`.  Species is named 
 * "A_<t>", the energy is ignored.
 */

void rm_add_species_ne(std::vector<species>& sp, size_t t) {
    stringstream ss;
    ss << "A_" << t;
	
    sp.push_back(species(sp.size(), ss.str(), false, 0));  
    sp.back().set_energy(0);
}


/*
 * Macro for adding a species to the vector `sp` without caring about
 * its energy. species gets name `name` and id is set accordingly.
 */

void rm_add_species_ne(std::vector<species>& sp, const std::string& name) {
    sp.push_back(species(sp.size(), name, false, 0));
    sp.back().set_energy(0);
}



/*
 * main
 */

int main(int argc, const char* argv[]) {
    srand(time(0));

    cl_para cl(argc, argv);  

   
    /*
     * Reads a jrnf-reaction network file and prints a textual
     * representation of the reactions. (The parameter 'in'
     * specifies which file to read)
     */   
   
    if(cl.have_param("print_network")){
        if(!cl.have_param("in")) {
	        cout << "You need to give parameter 'in'! Could not proceed!" << endl;
	        return 1;  
	    }
               
        std::vector<species> sp;  
	    std::vector<reaction> re;
      
    	std::string in=cl.get_param("in");
		
        if(read_jrnf_reaction_n(in, sp, re)) {
	        cout << "Error at reading jrnf-file!" << std::endl;  
	        return 1;
	    } else {
	        cout << "jrnf-File:" << endl;
	        for(size_t i=0; i<re.size(); ++i) 
	            cout << re[i].get_string(sp) << endl;
	    }
    }
    
    
    /*
     * Translates a jrnf file to a sbml file
     * ('in' gives input and 'out' output file)
     */
    
    if(cl.have_param("translate_jrnf_sbml")) {
        if(!cl.have_param("in") || !cl.have_param("out"))  {
	        cout << "You need to give parameters 'in' and 'out'! Could not proceed!" << endl;
	        return 1;  
	    }
	
	    cout << "Executing: translate_jrnf_sbml!" << endl;
	    std::string in=cl.get_param("in");
	    std::string out=cl.get_param("out");
	    std::vector<species> sp;
	    std::vector<reaction> re;
	
	    if(read_jrnf_reaction_n(in, sp, re)) {
	        cout << "Error at reading jrnf-file!" << std::endl;  
	        return 1;
	    }
	
	    cout << "Read file with " << sp.size() << " species and " << re.size() << " reactions!" << endl;
	    write_sbml_reaction_n(out, sp, re);	
    }
    
    
    /*
     * Transforms an reaction network from the file 'in' to the
     * file 'out' by removing all reactions with species 'sp'
     */
    
    if(cl.have_param("transform_rm_species_r")) {
        if(!cl.have_param("in") || !cl.have_param("out") || !cl.have_param("sp"))  {
	        cout << "You need to give parameters 'in', 'out' and 'sp'! Could not proceed!" << endl;
	        return 1;  
	    }      
      
      	cout << "Executing: transform_rm_species_r!" << endl;
	    cout << " (removing a species and all reactions with it)" << endl;
	    
	    std::string in=cl.get_param("in");
	    std::string out=cl.get_param("out");
	    std::string sp_name=cl.get_param("sp");
	    std::vector<species> sp, sp_out;
	    std::vector<reaction> re, re_out;
		
	    if(read_jrnf_reaction_n(in, sp, re)) {
	        cout << "Error at reading jrnf-file!" << std::endl;  
	        return 1;
	    }
	
	    filter_r_network_r(sp, re, sp_out, re_out, sp_name);
		
	    write_jrnf_reaction_n(out, sp_out, re_out);
    }

        
    /*
     * Transforms an reaction network from the file 'in' to the
     * file 'out' by removing all species 'sp' maintaining reduced
     * reactions
     */
    
    if(cl.have_param("transform_rm_species_s")) {
        if(!cl.have_param("in") || !cl.have_param("out") || !cl.have_param("sp"))  {
	        cout << "You need to give parameters 'in', 'out' and 'sp'! Could not proceed!" << endl;
	        return 1;  
	    }      
      
      	cout << "Executing: transform_rm_species_r!" << endl;
        cout << " (removing a species from network - keep reduced reactions)" << endl;
        std::string in=cl.get_param("in");
        std::string out=cl.get_param("out");
        std::string sp_name=cl.get_param("sp");
        std::vector<species> sp, sp_out;
        std::vector<reaction> re, re_out;
    
        if(read_jrnf_reaction_n(in, sp, re)) {
            cout << "Error at reading jrnf-file!" << std::endl;  
            return 1;
        }      
	
        filter_r_network_s(sp, re, sp_out, re_out, sp_name);
	
    	write_jrnf_reaction_n(out, sp_out, re_out);
    }   

    
    /*
     * Combines two networks (in1, in2) and write result to jrnf-file (out). 
     *
     */
    
    if(cl.have_param("combine_networks")) {
        std::cout << "mode: combine_networks" << std::endl;
      
        if(!cl.have_param("in1") || !cl.have_param("in2") || !cl.have_param("out"))  {
            cout << "You need to give parameters 'in1', 'in2' and 'out'! Could not proceed!" << endl;
            return 1;  
        }      

        std::vector<species> sp_1, sp_2, sp;  
        std::vector<reaction> re_1, re_2, re;
	
        std::string in1=cl.get_param("in1");
        std::string in2=cl.get_param("in2");
        
        if(read_jrnf_reaction_n(in1, sp_1, re_1) || read_jrnf_reaction_n(in2, sp_2, re_2)) {
            cout << "Error at reading jrnf-file!" << std::endl;  
            return 1;
        }     
	
        combine_r_networks(sp_1, re_1, sp_2, re_2, sp, re);
	
        cout << "Combined network having " << sp.size() << " species and " << re.size() << " reactions." << endl;
	
        std::string out=cl.get_param("out");
        cout << "Writing reaction network to " << out << endl;
        write_jrnf_reaction_n(out, sp, re);
    }

    
    /*
     * Create simple Erdos-Renyi-Network with given number of
     * nodes N and edges M. Possibility to choos if self loops
     * and multiple edges are possible. It's also possible to generate
     * directed networks.
     */
     
    if(cl.have_param("create_ER_NM")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "ER_NM_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
      
        std::cout << "mode: create_ER_NM  N=" << N << "   M=" << M << "   out=" << out << std::endl;
	    
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;
		
	std::cout << "creating network" << std::endl;
	
     	std::vector< pair<size_t, size_t> > edges;
        create_erdos_renyi(edges, N, M,allow_multiple,self_loop, directed);
	    
	vector<species> sp;
	vector<reaction> re;
		
        cout << "Simple output!" << std::endl;  
		    
	for(size_t t=0; t<N; ++t) 
	    rm_add_species_ne(sp, t);
		    
	for(size_t t=0; t<edges.size(); ++t) 
	    rm_1to1rev(re, edges[t].first, edges[t].second);  
            
	write_jrnf_reaction_n(out, sp, re);
    }
    
   
    /*
     * Create reaction network from Erdos-Renyi model coupling linear reactions to nonlinear
     */   
   
    if(cl.have_param("create_ER_NM_bi_C")) {
        size_t N=cl.get_param_i("N");
	size_t M=cl.get_param_i("M");
	size_t C=cl.get_param_i("C");
	std::string out= cl.have_param("out") ? cl.get_param("out") : "bi_nMC_network.jrnf";
	bool self_loop=cl.have_param("self_loop");
	bool directed=cl.have_param("directed");
	bool allow_multiple=cl.have_param("allow_multiple");
        bool limit_coupling=cl.have_param("limit_coupling");

	
	// Energy distribution of species and for activation energy
	// TODO Not implemented yet
	size_t energy_dist=0;   // 0 <-> linear [-1, 0]    
	                        // 1 <-> logarithmic ln([0.01,1])
	size_t aener_dist=0;    // 0 <-> linear [0, 1]
	                        // 1 <-> logarithmic -ln([0.01,1])
	
        std::cout << "mode: create_ER_NM_bi_C  N=" << N << "   M=";
        std::cout << M << "    C=" << C << "    out=" << out << std::endl;
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;

        if(limit_coupling)
            std::cout << "limit coupling is active!" << std::endl;

        cout << "Energy distribution is " << energy_dist;
        cout << " and activation energy dist is " << aener_dist << endl;
	
	std::cout << "creating network" << std::endl;
	
     	std::vector< pair<size_t, size_t> > edges, couples;
	create_erdos_renyi(edges, N, M, allow_multiple,self_loop, directed);
        couple_erdos_renyi(couples, C, edges, limit_coupling, allow_multiple, self_loop, directed);
	    
	vector<species> sp;
	vector<reaction> re;
		
        cout << "Output!" << std::endl;  

	for(size_t t=0; t<N; ++t) 
            rm_add_species(sp, t, energy_dist);    
		
        // Combine network link to "a+b->c+d"-reactions
        for(size_t i=0; i<couples.size(); ++i) {
            size_t r1=couples[i].first;
            size_t r2=couples[i].second;
		    
            size_t a(edges[r1].first), b(edges[r2].first), 
            c(edges[r1].second), d(edges[r2].second);
	   
            // create reaction using the macro function
            rm_2to2rev(re, a, b, c, d, aener_dist);
		    
            // removing links connected from the edge list
            edges.erase(edges.begin()+r2);
            edges.erase(edges.begin()+r1);		  
	}
		
    
        // "Translate" all those unary reactions ("A->B")
        for(size_t t=0; t<edges.size(); ++t) {
            size_t a(edges[t].first), b(edges[t].second);
			   
            // create reaction using the macro function
            rm_1to1rev(re, a, b, aener_dist);
        }

        write_jrnf_reaction_n(out, sp, re);	         
    }

   
    
    /*
     * Create Barabasi-Albert network with given number of edges and nodes.
     */
     
    if(cl.have_param("create_BA_NM")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
	    std::string out= cl.have_param("out") ? cl.get_param("out") : "BA_NM_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
      
        std::cout << "mode: create_BA_NM  N=" << N << "   M=" << M << "   out=" << out << std::endl;
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;
		
        std::cout << "creating network" << std::endl;
	
     	std::vector< pair<size_t, size_t> > edges;
        create_barabasi_albert(edges, N, M,allow_multiple,self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Simple output!" << std::endl;  
		    
        for(size_t t=0; t<N; ++t) 
            rm_add_species_ne(sp, t);
		    
        for(size_t t=0; t<edges.size(); ++t) 
            rm_1to1rev(re, edges[t].first, edges[t].second);  
            
        write_jrnf_reaction_n(out, sp, re);
    }
    
    
    /*
     * Create Barabasi-Albert network and couple linear reactions to nonlinear 
     */
    
    if(cl.have_param("create_BA_NM_bi_C")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
        size_t C=cl.get_param_i("C");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "bi_NMC_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
        bool limit_coupling=cl.have_param("limit_coupling");
	
        // Energy distribution of species and for activation energy
        // TODO Not implemented yet
        size_t energy_dist=0;   // 0 <-> linear [-1, 0]    
                                // 1 <-> logarithmic ln([0.01,1])
        size_t aener_dist=0;    // 0 <-> linear [0, 1]
                                // 1 <-> logarithmic -ln([0.01,1])
	
	       
        std::cout << "mode: create_BA_NM_bi_C  N=" << N << "   M=";
        std::cout << M << "    C=" << C << "    out=" << out << std::endl;
	
    	if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;

        if(limit_coupling)
            std::cout << "limit coupling is active!" << std::endl;

        cout << "Energy distribution is " << energy_dist;
        cout << " and activation energy dist is " << aener_dist << endl;
	

        std::cout << "creating network" << std::endl;

        std::vector< pair<size_t, size_t> > edges, couples;
        create_barabasi_albert(edges, N, M,allow_multiple,self_loop, directed);

	cout << "Doing coupling." << endl;
        couple_barabasi_albert(couples, C, edges, limit_coupling, allow_multiple, self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Output!" << std::endl;  

       for(size_t t=0; t<N; ++t) 
           rm_add_species(sp, t, energy_dist);    
		
        // Combine network link to "a+b->c+d"-reactions
        for(size_t i=0; i<couples.size(); ++i) {
            size_t r1=couples[i].first;
            size_t r2=couples[i].second;
		    
            size_t a(edges[r1].first), b(edges[r2].first), 
                   c(edges[r1].second), d(edges[r2].second);
			   
            // create reaction using the macro function
            rm_2to2rev(re, a, b, c, d, aener_dist);
		    
            // removing links connected from the edge list
            edges.erase(edges.begin()+r2);
            edges.erase(edges.begin()+r1);		  
        }
		
        // "Translate" all those unary reactions ("A->B")
        for(size_t t=0; t<edges.size(); ++t) {
            size_t a(edges[t].first), b(edges[t].second);
			   
            // create reaction using the macro function
            rm_1to1rev(re, a, b, aener_dist);
        }
			
        write_jrnf_reaction_n(out, sp, re);	         
    }

    
    /*
     * Create Watz-Strogatz network with given number of edges and nodes.
     */
     
    if(cl.have_param("create_WS_NMalpha")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
        double alpha=cl.get_param_d("alpha");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "WS_NMalpha_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
      
        std::cout << "mode: create_WS_NMalpha  N=" << N << "   M=" << M << "    alpha=" << alpha << "   out=" << out << std::endl;
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "multiple is active!" << std::endl;

        std::cout << "creating network" << std::endl;
	
        std::vector< pair<size_t, size_t> > edges;
        create_watts_strogatz(edges, N, M, alpha,allow_multiple,self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Simple output!" << std::endl;  
		    
        for(size_t t=0; t<N; ++t) 
    	    rm_add_species_ne(sp, t);
		    
        for(size_t t=0; t<edges.size(); ++t) 
            rm_1to1rev(re, edges[t].first, edges[t].second);  
            
        write_jrnf_reaction_n(out, sp, re);
    }
    
    
    /*
     * Create Watz-Strogatz network and couple linear reactions to nonlinear 
     */
    
    if(cl.have_param("create_WS_NMalpha_bi_C")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
        double alpha=cl.get_param_d("alpha");
    	size_t C=cl.get_param_i("C");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "bi_NMalphaC_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
        bool limit_coupling=cl.have_param("limit_coupling");
	
        // Energy distribution of species and for activation energy
        // TODO Not implemented yet
        size_t energy_dist=0;   // 0 <-> linear [-1, 0]    
                                // 1 <-> logarithmic ln([0.01,1])
        size_t aener_dist=0;    // 0 <-> linear [0, 1]
                                // 1 <-> logarithmic -ln([0.01,1])
	
        std::cout << "mode: create_WS_NMalpha_bi_C  N=" << N << "   M=";
        std::cout << M << "    alpha=" << alpha << "   C=" << C;
        std::cout << "    out=" << out << std::endl;
	
	    
    	if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;

        if(limit_coupling)
            std::cout << "limit coupling is active!" << std::endl;

        cout << "Energy distribution is " << energy_dist;
        cout << " and activation energy dist is " << aener_dist << endl;
	
        std::cout << "creating network" << std::endl;
	
        std::vector< pair<size_t, size_t> > edges, couples;
        create_watts_strogatz(edges, N, M, alpha,allow_multiple,self_loop, directed);
        couple_watts_strogatz(couples, C, edges, alpha, limit_coupling, allow_multiple, self_loop, directed);
	    
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Output!" << std::endl;  

        for(size_t t=0; t<N; ++t) 
            rm_add_species(sp, t, energy_dist);    
	
        // Combine network link to "a+b->c+d"-reactions
        for(size_t i=0; i<couples.size(); ++i) {
            size_t r1=couples[i].first;
            size_t r2=couples[i].second;
		    
            size_t a(edges[r1].first), b(edges[r2].first), 
                   c(edges[r1].second), d(edges[r2].second);
			   
            // create reaction using the macro function
            rm_2to2rev(re, a, b, c, d, aener_dist);
		    
            // removing links connected from the edge list
            edges.erase(edges.begin()+r2);
            edges.erase(edges.begin()+r1);		  
        }
		
    
        // "Translate" all those unary reactions ("A->B")
        for(size_t t=0; t<edges.size(); ++t) {
            size_t a(edges[t].first), b(edges[t].second);
			   
            // create reaction using the macro function
            rm_1to1rev(re, a, b, aener_dist);
        }
			
        write_jrnf_reaction_n(out, sp, re);	         
   }

    
   /*
    * Creates a hierarchical modular reaction network 
    */
     
    if(cl.have_param("create_PS_NMhmr")) {
        size_t N=cl.get_param_i("N");
        size_t M=cl.get_param_i("M");
        size_t m=cl.get_param_i("m");
        size_t h=cl.get_param_i("h");
        double r=cl.get_param_d("r");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "PS_NMhmr_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
      
        std::cout << "mode: create_PS_NMhmr  N=" << N << "   M=" << M << "    h=" << h << "   m=" << m << "   r=" << r << "   out=" << out << std::endl;
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
    	if(allow_multiple)
            std::cout << "multiple is active!" << std::endl;
	

        std::cout << "creating network" << std::endl;
	
     	std::vector< pair<size_t, size_t> > edges;
        create_pan_sinha(edges, N, M, h, m, r,allow_multiple,self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Simple output!" << std::endl;  
		    
        for(size_t t=0; t<N; ++t) 
            rm_add_species_ne(sp, t);
		    
        for(size_t t=0; t<edges.size(); ++t) 
            rm_1to1rev(re, edges[t].first, edges[t].second);  
            
        write_jrnf_reaction_n(out, sp, re);
    }


    /*
     * Creates a hierarchical modular network (Pan-Sinha model) and couples the linear 
     * reactions (A -> B) to a reaction network with nonlinear reactions.
     */
 
    if(cl.have_param("create_PS_NMhmr_bi_C")) {
        size_t N=cl.get_param_i("N");
    	size_t M=cl.get_param_i("M");
        size_t m=cl.get_param_i("m");
        size_t h=cl.get_param_i("h");
        double r=cl.get_param_d("r");
        size_t C=cl.get_param_i("C");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "PS_NMhmr_bi_C_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
        bool limit_coupling=cl.have_param("limit_coupling");
      
        std::cout << "mode: create_PS_NMhmr_bi_C   N=" << N << "   M=" << M;
        std::cout << "    h=" << h << "   m=" << m << "   r=" << r;
        std::cout << "  C=" << C << "   out=" << out << std::endl;
	
        // Energy distribution of species and for activation energy
        // TODO Not implemented yet
        size_t energy_dist=0;   // 0 <-> linear [-1, 0]    
                                // 1 <-> logarithmic ln([0.01,1])
        size_t aener_dist=0;    // 0 <-> linear [0, 1]
                                // 1 <-> logarithmic -ln([0.01,1])
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;

        if(limit_coupling)
            std::cout << "limit coupling is active!" << std::endl;

        cout << "Energy distribution is " << energy_dist;
        cout << " and activation energy dist is " << aener_dist << endl;

        std::vector< pair<size_t, size_t> > edges, couples;
        create_pan_sinha(edges, N, M, h, m, r,allow_multiple,self_loop, directed);
        couple_pan_sinha(couples, C, edges, h, m, r, limit_coupling, allow_multiple, self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Output!" << std::endl;  

        for(size_t t=0; t<N; ++t) 
    	    rm_add_species(sp, t, energy_dist);    
		
        // Combine network link to "a+b->c+d"-reactions
        for(size_t i=0; i<couples.size(); ++i) {
            size_t r1=couples[i].first;
            size_t r2=couples[i].second;
		    
            size_t a(edges[r1].first), b(edges[r2].first), 
                   c(edges[r1].second), d(edges[r2].second);
		   
            // create reaction using the macro function
            rm_2to2rev(re, a, b, c, d, aener_dist);
		    
            // removing links connected from the edge list
            edges.erase(edges.begin()+r2);
            edges.erase(edges.begin()+r1);		  
        }	
    
        // "Translate" all those unary reactions ("A->B")
        for(size_t t=0; t<edges.size(); ++t) {
            size_t a(edges[t].first), b(edges[t].second);
			   
            // create reaction using the macro function
            rm_1to1rev(re, a, b, aener_dist);
        }
			
        write_jrnf_reaction_n(out, sp, re);	         
    }


    /*
     * Creates a simple modular network  with <m> modules and couples the linear 
     * reactions (A -> B) to a reaction network with nonlinear reactions.
     */
 
    if(cl.have_param("create_SM_NMmr_bi_C")) {
        size_t N=cl.get_param_i("N");
    	size_t M=cl.get_param_i("M");
        size_t m=cl.get_param_i("m");
        double r=cl.get_param_d("r");
        size_t C=cl.get_param_i("C");
        std::string out= cl.have_param("out") ? cl.get_param("out") : "SM_NMmr_bi_C_network.jrnf";
        bool self_loop=cl.have_param("self_loop");
        bool directed=cl.have_param("directed");
        bool allow_multiple=cl.have_param("allow_multiple");
        bool limit_coupling=cl.have_param("limit_coupling");
      
        std::cout << "mode: create_PS_NMhmr_bi_C   N=" << N << "   M=" << M;
        std::cout << "   m=" << m << "   r=" << r;
        std::cout << "  C=" << C << "   out=" << out << std::endl;
	
	
        if(self_loop)
            std::cout << "self loop is active!" << std::endl;
	
        if(directed)
            std::cout << "directed is active!" << std::endl;
	
        if(allow_multiple)
            std::cout << "allow multiple is active!" << std::endl;

        if(limit_coupling)
            std::cout << "limit coupling is active!" << std::endl;

        std::vector< pair<size_t, size_t> > edges, couples;
        create_simple_modular(edges, N, M, m, r,allow_multiple,self_loop, directed);
        couple_simple_modular(couples, C, edges, m, r, limit_coupling, allow_multiple, self_loop, directed);
	    
        vector<species> sp;
        vector<reaction> re;
		
        cout << "Output!" << std::endl;  

        for(size_t t=0; t<N; ++t) 
    	    rm_add_species(sp, t, 0);    
		
        // Combine network link to "a+b->c+d"-reactions
        for(size_t i=0; i<couples.size(); ++i) {
            size_t r1=couples[i].first;
            size_t r2=couples[i].second;
		    
            size_t a(edges[r1].first), b(edges[r2].first), 
                   c(edges[r1].second), d(edges[r2].second);
		   
            // create reaction using the macro function
            rm_2to2rev(re, a, b, c, d, 0);
		    
            // removing links connected from the edge list
            edges.erase(edges.begin()+r2);
            edges.erase(edges.begin()+r1);		  
        }	
    
        // "Translate" all those unary reactions ("A->B")
        for(size_t t=0; t<edges.size(); ++t) {
            size_t a(edges[t].first), b(edges[t].second);
			   
            // create reaction using the macro function
            rm_1to1rev(re, a, b, 0);
        }
			
        write_jrnf_reaction_n(out, sp, re);	         
    }
    
    
    /*
     * Output of usage instructions by calling program with parameter 'help' or 'info'
     */
    
    if(cl.have_param("help") || cl.have_param("info")) {
        cout << "          Network tools" << endl;  
        cout << "          ===============" << endl;
        cout << " call with parameter 'info' or 'help' for showing this screen" << endl;
        cout << endl;
        cout << "-> print_network" << endl;
        cout << " Load a jrnf-file and print its reactions to the screen" << endl;
        cout << " --> in - Name of jrnf-file to print" << endl;
        cout << endl;
        cout << "-> translate_jrnf_sbml" << endl;
        cout << " Reads a jrnf-file and writes it as sbml" << endl;
        cout << " --> in - input file" << endl;
        cout << " --> out - output file" << endl;
        cout << endl;
        cout << "-> transform_rm_species_r, transform_rm_species_s" << endl;
        cout << " Transforms a reaction network, removing on species. Either all" << endl;
        cout << " reactions containing the species are removed ('_r') or only" << endl;
        cout << " the species is removed from these reactions ('_s')." << endl;
        cout << " --> in - input file" << endl;
        cout << " --> out - output file" << endl;
        cout << " --> sp - name of the species to be removed" << endl;
        cout << endl;
        cout << "-> create_ER_NM, create_BA_NM, create_WS_NMbeta, create_PS_NMhmr " << endl;
        cout << "-> create_ER_NM_bi_C, create_BA_NM_bi_C, create_WS_NMbeta_biC," << endl;
        cout << "-> create_PS_NMhmr_bi_C " << endl;
        cout << " Creates an Erdos Renyi, Barabasi Albert, Watz Strogatz" << endl;
        cout << " or Pan Sinha network (possible with coupling C edges)" << endl;
        cout << " reactions are reversible (A <-> B type)." << endl;
        cout << " --> N - Number of edges" << endl;
        cout << " --> M - Number of links" << endl;
        cout << " --> C - Number of link-pairs that are coupled" << endl;
        cout << " --> out - Output filename" << endl;
        cout << " --> self_loop - allow self loops (if possible)" << endl;
        cout << " --> directed - generate directed network" << endl;
        cout << " --> allow_multiple - allow multiple occurence of link" << endl;
        cout << " --> limit_coupling - coupling linear reactions with model specific constraints" << endl;
        cout << " --> beta - parameter for Watz Strogatz model" << endl;
        cout << " --> h - number of upper hierarchic level (PS)" << endl;
        cout << " --> m - size of 2. level modules (PS)" << endl;
        cout << " --> r - decrease of connectivity per level (PS)" << endl;
        cout << endl;
    }    
    
      
    return 0;
}
