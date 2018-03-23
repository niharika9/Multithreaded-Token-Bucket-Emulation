#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "cs402.h"
#include "my402list.h"

 int  My402ListLength(My402List* list)
{ 
	return list->num_members;
}
 int  My402ListEmpty(My402List* list)
{
	 return list->num_members<=0; 
}

 int  My402ListAppend(My402List* list, void* val)
{
 	   
  My402ListElem *node = (My402ListElem*)malloc(sizeof(My402ListElem));     
       if(My402ListEmpty(list))
       {
       	 node->obj=val;
       	 list->anchor.next=node;
       	 list->anchor.prev=node;
       	 node->prev=&list->anchor;
       	 node->next=&list->anchor;
       	 list->num_members++;
       	 return TRUE;      	
       }
       else
       {
       	node->obj=val;
       	node->prev = list->anchor.prev;
       	node->next = &list->anchor;
       	node->prev->next=node;
       	list->anchor.prev = node;
        list->num_members++;
        return TRUE;
       }     
}
 int  My402ListPrepend(My402List* list, void* val)
{
       
       My402ListElem *node = (My402ListElem*)malloc(sizeof(My402ListElem));
       if(My402ListEmpty(list))
       {
       	  node->obj=val;

       	  node->next=&list->anchor;
       	  node->prev=&list->anchor;
       	  list->anchor.prev=node;
       	  list->anchor.next=node;       	  
       	  list->num_members++;
       	  return TRUE;
                 }
       else
       {
       	   node->obj=val;
           node->next=list->anchor.next;
           node->prev=&list->anchor;
           list->anchor.next->prev=node;
           list->anchor.next=node;
           list->num_members++;
           return TRUE;
       }
 }
 void My402ListUnlink(My402List* list, My402ListElem* ptr)
{ 
     
 	ptr->next->prev=ptr->prev;
 	ptr->prev->next=ptr->next;
 	list->num_members--;
	free(ptr);

}
 void My402ListUnlinkAll(My402List* list)
{
        while(!My402ListEmpty(list))
        {
           My402ListUnlink(list,My402ListFirst(list));

        }
}     
 
 int  My402ListInsertAfter(My402List* list, void* val, My402ListElem* ptr)
{
      if(ptr==NULL) 
      	{
      		My402ListAppend(list,val);
      		list->num_members++;
      		return TRUE;
      }
      else
      {
      My402ListElem *node = (My402ListElem*)malloc(sizeof(My402ListElem));

      node->obj=val;
      node->next=ptr->next;
      node->prev=ptr;
      ptr->next->prev=node;
      ptr->next=node;
      list->num_members++;
      return TRUE;
      }
 }
 int  My402ListInsertBefore(My402List* list, void* val, My402ListElem* ptr)
{
 	if(ptr==NULL) 
      	{
      		My402ListPrepend(list,val);
      		list->num_members++;
      		return TRUE;
      }
      else
      {
      My402ListElem *node = (My402ListElem*)malloc(sizeof(My402ListElem));
      node->obj=val;
      node->next=ptr;
      node->prev=ptr->prev;
      ptr->prev->next=node;
      ptr->prev=node;
      list->num_members++;
      return TRUE;
    }

}

 My402ListElem *My402ListFirst(My402List* list)
 {
          if (My402ListEmpty(list))
          {
          	return NULL;
          }
          else return list->anchor.next;
 }

 My402ListElem *My402ListLast(My402List* list)
 {
          if (My402ListEmpty(list))
          {
          	return NULL;
          }
          else return list->anchor.prev;
 }
          
 My402ListElem *My402ListNext(My402List* list, My402ListElem* ptr)
 {
       if (ptr==My402ListLast(list))
       {
       	return NULL;
       }
 	   else 
	return ptr->next;	
  }
 My402ListElem *My402ListPrev(My402List* list, My402ListElem* ptr)
 {
    if(ptr==My402ListFirst(list))
 	return NULL;
     else 
  	return ptr->prev;
 }

 My402ListElem *My402ListFind(My402List* list, void* val)
{         
         My402ListElem *ptr=NULL;
        for (ptr=My402ListFirst(list);ptr!= NULL;ptr=My402ListNext(list, ptr)) 
	{
           if(ptr->obj==val) 
           	return ptr;  
         }
      return ptr; 
}

 int My402ListInit(My402List* list)
{
   list->num_members=0;
   list->anchor.next=&(list->anchor);
   list->anchor.prev=&(list->anchor);
   list->anchor.obj=NULL;
   return TRUE;
}
