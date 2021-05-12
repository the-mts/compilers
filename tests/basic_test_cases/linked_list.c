// Dynamic Memory, Ternary with Inbuilt Function Call 
// Passing Pointers (of Structs) as Arguments and Struct Pointers (Multi-level also)
struct Node
{
    int key;
    struct Node* next;
};
 
void push(struct Node** head_ref, int new_key)
{
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
 
    new_node->key  = new_key;

    new_node->next = (*head_ref);

    (*head_ref)    = new_node;
}
 
int search(struct Node* head, int x)
{
    struct Node* current = head;
    while (current != 0)
    {
        if (current->key == x)
            return 1;
        current = current->next;
    }
    return 0;
}

int main()
{

    struct Node* head = 0;
    
    int n;
    int i;
    int tmp;
    int x;
    int cont = 1;

    printf("Enter the number of nodes in the linked list: ");
    scanf("%d", &n);
    printf("Enter %d numbers: \n", n);

    for(i = 0; i < n ; i++){
        scanf("%d", &tmp);
        push(&head, tmp);
    }
    while(cont){
        printf("Which number do you want to search in the linked list?\n");
        scanf("%d", &x);
        search(head, x)? printf("Yes, %d is present in the linked list.\n", x) : printf("No, %d is not present in the linked list.\n", x);
        printf("Want to continue? (1/0)\n");
        scanf("%d", &cont);
    }

    return 0;
}