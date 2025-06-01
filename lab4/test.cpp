#include<bits/stdc++.h>
using namespace std;

struct TreeNode{
    TreeNode* left;
    TreeNode* right;
    int val;
};

struct GraphNode{
    int inDegree;
    int outDegree;
    int val;
    vector<GraphNode*> outNodes;
    vector<GraphNode*> inNodes;
};

int main() {
    int result = false;
    vector<GraphNode*> graph;
    
    TreeNode* root;

    // 排除非二叉的情况
    for(auto node : graph) {
        if(node->outDegree >= 2) {
            return result;
        }
    }

    // 将所有图节点转化为TreeNode



    // 找到二叉树的根节点
    for(int i=0; i<graph.size(); i++) {
        GraphNode* curNode = graph[i];
        if(curNode->inDegree == 0) {
            root->val = curNode->val;
            if(curNode->outDegree>=2) {
                return result;
            }
            for(int j = 0; j < curNode->outNodes.size(); j++) {
                if(j == 0) {
                    root->left = new TreeNode();
                    root->left->val = curNode->outNodes[j]->val;
                }
                else {
                    root-> right = new TreeNode();
                    root->right->val = curNode->outNodes[j]->val;
                }
            }
        }
    }

    // 基于根节点，构建二叉树
    


}