// Dynamic Memory, Passing Pointers (of Structs) as Arguments and Struct Pointers (Multi-level also)
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
    int x = 21; // Should Give Yes.
    // int x = 20; // Should Give No.
 
    push(&head, 10);
    push(&head, 30);
    push(&head, 11);
    push(&head, 21);
    push(&head, 14);
 
    search(head, x)? printf("Yes, %d is present in the linked list.\n", x) : printf("No, %d is not present in the linked list.\n", x);
    return 0;
}