#include "DDG.h"

DataDepEdge::DataDepEdge(DataDepNode *src, DataDepNode *dst) {
    src_node = src;
    dst_node = dst;
    distance = 0;  // default distance is 0
}

unsigned DataDepNode::DDBG_Height(DataDepNode *node) {
    if (node->get_color() == Color::BLACK) {
        return 0;
    } else {
        node->set_color(Color::BLACK);  // visit the node
    }
    unsigned max_height = 0;
    unsigned current_height = 0;
    DataDepNode *max_height_user_node = node;
    Instruction *inst = node->get();
    //errs() << "Calculate the height of instruction:\n" << *inst << "\n";
    for (iterator i = node->user_edge_begin(), i_end = node->user_edge_end(); i != i_end; i++) {
        DataDepEdge *user_edge = *i;
        DataDepNode *user_node = user_edge->get_dst_node();
        //current_height = DDBG_Height(user_node) + user_edge->get_edge_latency();
        current_height = DDBG_Height(user_node) + node->get_latency();
        if (max_height < current_height) {
            max_height = current_height;
            max_height_user_node = user_node; 
        }
    }
    node->set_color(Color::WHITE);  // release the visited child node
    if (node->user_edge_begin() == node->user_edge_end()) { // the leaf node (update this to constrain the node within same ddbg)
        return (max_height + 1); // the exit node (update node latency)
    }
    else {
        return max_height;
    }
    

}

DataDepNode::DataDepNode(Instruction *inst, DataDepBasicGraph *ddbg) {
    self = inst;
    parent = ddbg;
    set_color(Color::WHITE);
    ddbg_height = 0;
    schedule = 0;
    latency = 1; // update
}

void DataDepNode::ConstructEdges() {
    Instruction *inst = self;
    errs() << "    --for instruction" << *inst << "\n";
    unsigned op_num;
    errs() << "      The USEs are: \n";
    // get the use_edges
    op_num = 0;
    for (Use *u = inst->op_begin(), *u_end = inst->op_end(); u != u_end; u++ ) {
        Instruction *use_inst = dyn_cast<Instruction>(u->get());
        DataDepNode *src = parent->get_node(use_inst);
        if (src) {
            DataDepEdge *use_edge_new = new DataDepEdge(src ,this);
            use_edges.push_back(use_edge_new);
            errs() << "        " << *(src->get()) << "\n";
        } else {
            errs() << "        no use found for " << op_num << "-th operand" << "\n";
        }
        op_num++;
    }

    errs() << "      The USERs are: \n";
    // get the user_edges
    for (Value::use_iterator u = inst->use_begin(), u_end = inst->use_end(); u != u_end; u++ ) {
        Instruction *user_inst = dyn_cast<Instruction>(*u);
        DataDepNode *dst = parent->get_node(user_inst);
        if (dst) {
            DataDepEdge *user_edge_new = new DataDepEdge(this, dst);
            user_edges.push_back(user_edge_new);
            errs() << "        " << *(dst->get()) << "\n";
        }
    }
}

void DataDepNode::add_use_edge(DataDepNode *src, DataDepNode *dst, unsigned distance) {
    DataDepEdge *use_edge_new = new DataDepEdge(src, dst);
    use_edge_new->set_distance(distance);
    use_edges.push_back(use_edge_new);
}

void DataDepNode::add_user_edge(DataDepNode *src, DataDepNode *dst, unsigned distance) {
    DataDepEdge *user_edge_new = new DataDepEdge(src, dst);
    user_edge_new->set_distance(distance);
    user_edges.push_back(user_edge_new);
}

DataDepBasicGraph::DataDepBasicGraph(BasicBlock *bb) {
    // initialized each node with self and parent
    unsigned num = 0;
    self = bb;
    for (BasicBlock::iterator inst = bb->begin(), inst_end = bb->end(); inst != inst_end; ++inst) {

        DataDepNode *node_new = new DataDepNode(inst, this);
        nodes.push_back(node_new);
        pair<Instruction*, DataDepNode*> pair_new (inst, node_new);
        ddbg_map.insert(pair_new);
        num++;
    }

    // initialize use/user edges for each node
    
    for (vector<DataDepNode*>::iterator i = nodes.begin(), i_end = nodes.end(); i != i_end; i++) {
        (*i)->ConstructEdges();
    }

    // initialize height for each node
    errs() << "Calculate the height of instruction:\n";
    for (iterator i = nodes.begin(), i_end = nodes.end(); i != i_end; i++) {
        errs() << *((*i)->get()) << "\n";
        unsigned h = (*i)->DDBG_Height(*i);
        errs() << "    h = " << h << "\n";
        (*i)->set_ddbg_height(h);
    }
}

void DataDepBasicGraph::calculate_distance(DependenceAnalysis *DA) {
    typedef SmallVector<Value *, 16> ValueVector;
    ValueVector MemInstr;
    unsigned inst_num = 0;
    for (BasicBlock::iterator I = self->begin(), E = self->end(); I != E; ++I) {
        Instruction *Ins = dyn_cast<Instruction>(I);
        if (!Ins)
            continue;
        LoadInst *Ld = dyn_cast<LoadInst>(I);
        StoreInst *St = dyn_cast<StoreInst>(I);
        if (!St && !Ld)
            continue;
        if (Ld && !Ld->isSimple())
            continue;
        if (St && !St->isSimple())
            continue;
        inst_num++;
        MemInstr.push_back(&*I);
        errs() << "  MemInst-" << inst_num << ":" << *I << "\n";
    }

    ValueVector::iterator I, IE, J, JE;
    // analyze iteration distance
    for (I = MemInstr.begin(), IE = MemInstr.end(); I != IE; ++I) {
        for (J = I, JE = MemInstr.end(); J != JE; ++J) {
            Instruction *Src = dyn_cast<Instruction>(*I);
            Instruction *Des = dyn_cast<Instruction>(*J);
            if (Src == Des)
                continue;
            if (isa<LoadInst>(Src) && isa<LoadInst>(Des))
                continue;
            if (auto D = DA->depends(Src, Des, true)) {
                if (D->isFlow()) {
                    errs () << "Flow dependence not handled";
                    continue;
                }
                if (D->isAnti()) {
                    unsigned Levels = D->getLevels();
                    for (unsigned II = 1; II <= Levels; ++II) {  // update: how to deal with multi-level loop?
                        const SCEV *Distance = D->getDistance(II);
                        const SCEVConstant *SCEVConst = dyn_cast_or_null<SCEVConstant>(Distance);
                        if (SCEVConst) {
                            const ConstantInt *CI = SCEVConst->getValue();
                            unsigned it_dist = abs(CI->getUniqueInteger().getSExtValue());
                            // add new edges: src->use_edge & des->user_edge
                            DataDepNode *src_node = ddbg_map[Src];
                            DataDepNode *des_node = ddbg_map[Des];
                            src_node->add_use_edge(des_node, src_node, it_dist);
                            src_node->add_use_edge(des_node, src_node, it_dist);
                            errs() << "Found Anti dependence between:\n  Src:" << *Src << "\n  Des:" << *Des
                     << "\n";
                            errs() << "  level = " << II << ", distance = "<< it_dist << "\n";
                        } 
                    }
                }
            }
        }
    }
}

DataDepGraph::DataDepGraph(Function *func, DependenceAnalysis *DA) {
    // initialize each bb in function
    self = func;
    errs() << "--for function "<< func->getName() << ":\n";
    unsigned bb_num = 0;
    for (Function::iterator bb = func->begin(), bb_end = func->end(); bb != bb_end; bb++) {
        bb_num += 1;
        errs() << "  --for BB-"<< bb_num << ":\n";
        DataDepBasicGraph *ddbg_new = new DataDepBasicGraph(bb);
        ddg.push_back(ddbg_new);
        pair<BasicBlock*, DataDepBasicGraph*> pair_new (bb, ddbg_new);
        ddg_map.insert(pair_new);
    }

    // update loop-carried distance for each ddbg
    bb_num = 0;
    for (vector<DataDepBasicGraph*>::iterator i = ddg.begin(), i_end = ddg.end(); i != i_end; i++) {
        bb_num += 1;
        errs() << "  --for BB-"<< bb_num << ":\n";
        (*i)->calculate_distance(DA);
    }

    // schedule for each ddbg
    bb_num = 0;
    for (vector<DataDepBasicGraph*>::iterator i = ddg.begin(), i_end = ddg.end(); i != i_end; i++) {
        bb_num += 1;
        errs() << "--------------Sorted Priority Queue--------------------\n";
        errs() << "---BB-"<< bb_num << ":\n";
        map<ResFPGA,unsigned> rescon;
        rescon[LUT] = 10;  // update
        rescon[FF] = 10;
        rescon[BRAM] = 10;
        rescon[DSP] = 10;

        ListScheduler list_scheduler(*i, rescon);
    }
}

bool isHeigher(DataDepNode *entry_1, DataDepNode *entry_2) {
    unsigned h_1 = entry_1->get_ddbg_height();
    unsigned h_2 = entry_2->get_ddbg_height();
    return h_1 > h_2;
}

ListScheduler::ListScheduler(DataDepBasicGraph* ddbg, map<ResFPGA,unsigned> &ResCon) {    
    // construct priority queue
    for (vector<DataDepNode*>::iterator i = ddbg->begin(), i_end = ddbg->end(); i != i_end; i++) {
        queue.push_back(*i);
        //unsigned h = (*i)->get_ddbg_height();
    }

    // sort priority queue, heighest to lowest
    sort(queue.begin(), queue.end(), isHeigher);

    // begin to schedule
     // iterate over nodes in priority order
    for (vector<DataDepNode*>::iterator i = queue.begin(), i_end = queue.end(); i < i_end; i++) {
        errs() << "h = " << (*i)->get_ddbg_height() << "  "; 
        // iterate through all uses
        unsigned s_max = 0, s = 0;
        for (vector<DataDepEdge*>::iterator i_edge = (*i)->use_edge_begin(), i_edge_end = (*i)->use_edge_end(); i_edge != i_edge_end; i_edge++) {
            // find the earliest schedulable time slot
            DataDepNode *node = (*i_edge)->get_src_node();
            if (node) {
                unsigned s_pre = node->get_schedule();
                //unsigned d_pre = (*i_edge)->get_edge_latency();
                unsigned d_pre = node->get_latency();
                unsigned s_now = s_pre + d_pre;
                if (s_max < s_now) {
                    s_max = s_now;
                }
            }
        }
        s = s_max;

        // check resource constaint (update)
        while (0) {
            s += 1;
        }
        (*i)->set_schedule(s);

        // update ResTable (Resource Consumption Table)
        errs() << "s = " << (*i)->get_schedule() << "\n"; 
    }
}