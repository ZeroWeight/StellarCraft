#define _OJ_
#include <iostream>
 
class TreeNode {
public:
    TreeNode() {
        lc = rc = parent = NULL;
    }
    TreeNode(int val, int level, TreeNode *parent) : val(val), level(level), parent(parent) {
        lc = rc = NULL;
    }
 
    int val, level;
    TreeNode *lc, *rc, *parent;
};
 
void insert(TreeNode *root, const int &val) {
    if (val < root->val) {
        if (root->lc == NULL) root->lc = new TreeNode(val, root->level + 1, root);
        else insert(root->lc, val);
    }
    else {
        if (root->rc == NULL) root->rc = new TreeNode(val, root->level + 1, root);
        else insert(root->rc, val);
    }
}
 
TreeNode *locate(TreeNode *root, const int &val) {
    if (root->val == val) return root;
    if (val < root->val) return locate(root->lc, val);
    else return locate(root->rc, val);
}
 
int main() {
    #ifndef _OJ_
    freopen("test.in", "r", stdin);
    #endif
 
    int n, m;
    std::cin >> n >> m;
 
    TreeNode *root = NULL;
    for (int i = 0; i < n; ++i) {
        int x;
        std::cin >> x;
        if (root == NULL) root = new TreeNode(x, 0, NULL);
        else insert(root, x);
    }
 
    for (int i = 0; i < m; ++i) {
        int x, y;
        std::cin >> x >> y;
 
        TreeNode *xNode = locate(root, x);
        TreeNode *yNode = locate(root, y);
        while (xNode->level > yNode->level) xNode = xNode->parent;
        while (yNode->level > xNode->level) yNode = yNode->parent;
        while (xNode != yNode) xNode = xNode->parent, yNode = yNode->parent;
        std::cout << xNode->val << std::endl;
    }
 
    #ifndef _OJ_
    fclose(stdin);
    #endif
 
    return 0;
}
/**************************************************************
    Problem: 1021
    User: 2015011481
    Language: C++
    Result: 正确
    Time:81 ms
    Memory:3744 kb
****************************************************************/
