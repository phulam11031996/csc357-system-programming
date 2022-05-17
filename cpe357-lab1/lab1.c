#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct listelement
{
  struct listelement *next, *prev;
  char text[100];
} listelement;

/* function declaration */
void push_string(listelement **head);
void print_list(listelement *head);
void delete_item(listelement **head);
void end_program(listelement *head);

int main()
{
  printf("Select:\n");
  listelement *head = NULL;

  for (;;)
  {
    printf("Select:\n");
    printf("1. push string\n");
    printf("2. print list\n");
    printf("3. delete item\n");
    printf("4. end program\n");
    printf(">");
    int choice;
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
      push_string(&head);
      break;
    case 2:
      print_list(head);
      break;
    case 3:
      delete_item(&head);
      break;
    default:
      end_program(head);
      return 0;
      break;
    }
  }

  return 0;
}

/* function definition */
void push_string(listelement **head)
{
  printf("insert text\n");
  char charInput[100];
  scanf("%s", charInput);

  if (*head == NULL)
  {
    *head = (listelement *)malloc(sizeof(listelement));
    strcpy((*head)->text, charInput);
    (*head)->next = NULL;
    (*head)->prev = NULL;
  }
  else
  {
    listelement *newElement = (listelement *)malloc(sizeof(listelement));
    listelement *last = NULL;
    for (last = *head; last->next != NULL; last = last->next)
    {
    }
    strcpy(newElement->text, charInput);
    last->next = newElement;
    newElement->prev = last;
    newElement->next = NULL;
  }

  printf("done push string \n");
}

void print_list(listelement *head)
{
  for (listelement *temp = head; temp != NULL; temp = temp->next)
  {
    printf("%s\n", temp->text);
  }
}

void delete_item(listelement **head)
{
  printf("insert index\n");
  int idx;
  scanf("%d", &idx);

  if (*head == NULL)
  {
    printf("invalid index\n");
    return;
  }
  else if (idx == 0)
  {
    listelement *pTemp = *head;
    *head = (*head)->next;
    free(pTemp);
  }
  else
  {
    listelement *pTemp = *head;
    for (int i = 0; i != idx; i++)
    {
      if (pTemp->next)
      {
        pTemp = pTemp->next;
      }
      else
      {
        printf("invalid index\n");
        return;
      }
    }

    listelement *pA = pTemp->prev;
    listelement *pC = pTemp->next;
    if (pA)
    {
      pA->next = pC;
    }
    if (pC)
    {
      pC->prev = pA;
    }
    free(pTemp);
  }
}

void end_program(listelement *head)
{
  listelement *temp = head;
  while (head)
  {
    temp = head;
    head = head->next;
    free(temp);
  }
  printf("program ended\n");
}
