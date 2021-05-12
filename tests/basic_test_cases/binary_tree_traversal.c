// Dynamic Memory, Returning Pointer to a Struct, Tail Call Optimization
struct node {
    int data;
    struct node* left;
    struct node* right;
};
 

struct node* newNode(int data)
{
    struct node* node = (struct node*)malloc(sizeof(struct node));
    node->data = data;
    node->left = 0;
    node->right = 0;
 
    return (node);
}
 

void printPostorder(struct node* node)
{
    if (node == 0)
        return;
 

    printPostorder(node->left);
 
    printPostorder(node->right);
 
    printf("%d ", node->data);
}
 
void printInorder(struct node* node)
{
    if (node == 0)
        return;
 
    printInorder(node->left);
 
    printf("%d ", node->data);
 
    printInorder(node->right);
}
 
void printPreorder(struct node* node)
{
    if (node == 0)
        return;
 
    printf("%d ", node->data);

    printPreorder(node->left);

    printPreorder(node->right);
}

int main()
{
    struct node* root = newNode(1);
    root->left = newNode(2);
    root->right = newNode(3);
    root->left->left = newNode(4);
    root->left->right = newNode(5);
    root->right->left = newNode(6);
    root->right->right = newNode(7);
 
    printf("\nPreorder traversal of binary tree is \n");
    printPreorder(root);
 
    printf("\nInorder traversal of binary tree is \n");
    printInorder(root);
 
    printf("\nPostorder traversal of binary tree is \n");
    printPostorder(root);
    printf("\n");

    return 0;
}
/*    1
   2     3
 4   5 6   7  
 */