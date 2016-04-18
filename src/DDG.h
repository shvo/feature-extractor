#ifndef DDG_H
#define DDG_H

#include <algorithm>
#include <map>
#include <iostream>
#include <stdlib.h>

#include "llvm/IR/Instruction.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

using namespace std;
using namespace llvm;

class DataDepNode;
class DataDepBasicGraph;
class ListScheduler;
enum ResFPGA {LUT, FF, BRAM, DSP};

class DataDepEdge {
private:
    DataDepNode *src_node;
    DataDepNode *dst_node;
    unsigned distance;
    //unsigned edge_latency;  // data dependency latency
    //unsigned edge_distance;
public:
    DataDepEdge(DataDepNode*, DataDepNode*);
    DataDepNode* get_src_node() {return src_node;}
    DataDepNode* get_dst_node() {return dst_node;}
    void set_distance(unsigned dist) {distance = dist;}
    unsigned get_distance() {return distance;}
};


class DataDepNode {
private:
    typedef vector<DataDepEdge*>::iterator iterator;
    enum Color {BLACK, WHITE};
    //enum ResFPGA {LUT, FF, BRAM, DSP};  // update mem access ports
    
    Instruction *self;
    DataDepBasicGraph *parent;
    Color color;
    unsigned ddbg_height;
    vector<DataDepEdge*> use_edges;  // use edges
    vector<DataDepEdge*> user_edges;  // user edges
    unsigned schedule;  // schedule within ddbg
    map<ResFPGA,unsigned> resource;  // FPGA resource consumption of this node (opcode + variable)
    unsigned latency;
public:
    DataDepNode(Instruction*, DataDepBasicGraph*);
    void ConstructEdges();
    Instruction* get() {return self;}
    Color get_color() {return color;}
    void set_color(Color c) {color = c;}
    unsigned get_ddbg_height() {return ddbg_height;}
    void set_ddbg_height(unsigned h) {ddbg_height = h;}
    unsigned DDBG_Height(DataDepNode*);
    DataDepBasicGraph* get_parent() {return parent;}
    void add_use_edge(DataDepNode*, DataDepNode*, unsigned);
    void add_user_edge(DataDepNode*, DataDepNode*, unsigned);
    iterator use_edge_begin() {return use_edges.begin();}
    iterator use_edge_end() {return use_edges.end();}
    iterator user_edge_begin() {return user_edges.begin();}
    iterator user_edge_end() {return user_edges.end();}
    unsigned get_schedule() {return schedule;}
    void set_schedule(unsigned s) {schedule = s;}
    map<ResFPGA,unsigned> get_res() {return resource;}
    unsigned get_latency() {return latency;}
    //void set_res(DenseMap<ResFPGA,unsigned> r) {res = r;}
};

class DataDepBasicGraph {
private:
    typedef vector<DataDepNode*>::iterator iterator;

    DenseMap<Instruction*, DataDepNode*> ddbg_map; // instruction to node
    vector<DataDepNode*> nodes;  // nodes of a basic block ddg
    BasicBlock *self;
public:
    DataDepBasicGraph(BasicBlock*);
    BasicBlock* get() {return self;}
    DataDepNode* get_node(Instruction* inst) {
        auto found = (ddbg_map.lookup(inst));
        return found;
    }
    iterator begin() {return nodes.begin();}
    iterator end() {return nodes.end();}
    void calculate_distance(DependenceAnalysis*); // calculate loop-carried iteration distance
};

class DataDepGraph {
private:
    DenseMap<BasicBlock*, DataDepBasicGraph*> ddg_map; // basic block to basic block ddg
    vector<DataDepBasicGraph*> ddg; // ddgs of basic blocks
    Function *self;
    typedef vector<DataDepBasicGraph*>::iterator iterator;
public:
    DataDepGraph(Function*, DependenceAnalysis*);
    DataDepBasicGraph* get_graph(BasicBlock* bb) {
        auto found = (ddg_map.find(bb));
        return found->second;
    }
    iterator begin() {return ddg.begin();}
    iterator end() {return ddg.end();}
};

class ListScheduler {   // list scheduling for each basic block
//public:    
//    enum ResFPGA {LUT, FF, BRAM, DSP};  // update mem access ports
private:
    typedef map<ResFPGA,unsigned> ResNum;
    //ResNum ResCon;  // resource constaint
    vector<ResNum*> ResTable;  // resource table (update)
    vector<DataDepNode*> queue;  // data-dependency height 
public:
    ListScheduler(DataDepBasicGraph*, ResNum&);
};

#endif