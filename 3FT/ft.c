#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "nodeFT.h"
#include "path.h"
#include "ft.h"
/*change to something #include "checkerDT.h"*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;

static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest);
static int FT_findNode(const char *pcPath, Node_T *poNResult, int nodeType);

int FT_insertDir(const char *pcPath){
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;
  
   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   if(oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 0, NULL, NULL);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /*we dont need to use any checker right*/
         /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

boolean FT_containsDir(const char *pcPath){
  int iStatus;
  Node_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findNode(pcPath, &oNFound, 0);
  return (boolean) (iStatus == SUCCESS);
}

int FT_rmDir(const char *pcPath){
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount))*/

   iStatus = FT_findNode(pcPath, &oNFound, 0);

   if(iStatus != SUCCESS)
       return iStatus;

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength){
   /*can pvcontents and ullength be NULL?*/
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= DT_traversePath(oPPath, &oNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   /*file cant be a root right?*/
   if(oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;

}

boolean FT_containsFile(const char *pcPath){
   int iStatus;
  Node_T oNFound = NULL;

  assert(pcPath != NULL);

  iStatus = FT_findNode(pcPath, &oNFound, 1);
  return (boolean) (iStatus == SUCCESS);
}

int FT_rmFile(const char *pcPath){
      int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount))*/

   iStatus = FT_findNode(pcPath, &oNFound, 1);

   if(iStatus != SUCCESS)
       return iStatus;

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

void *FT_getFileContents(const char *pcPath){
  Node_T oNFound = NULL;
  int iStatus;

  iStatus = FT_traversePath(pcPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      return iStatus;
   }
   if(oNFound->type == 1) return oNFound->contents;
   return FALSE;
}

void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength){
  Node_T oNFound = NULL;
  void *oldContent = NULL;

  int iStatus;

  iStatus = FT_traversePath(pcPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      return iStatus;
   }
   if(oNFound->type == 1){
    oldContent = oNFound->contents;
    oNFound->content = pvNewContents;
    oNFound->contentSize = ulNewLength;
   }
   return FALSE;
}

/*go through this and previous make sure it test for bugs as specified by the 
.h file */
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize){
  Node_T oNFound = NULL;
  int iStatus;

  iStatus = FT_traversePath(pcPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      return iStatus;
   }
   if(oNFound->type == 0){
    pbIsFile = FALSE;
    return SUCCESS;
   }
   else if(oNFound->type == 1){
    pbIsFile = TRUE;
    pulSize = oNFound->contentSize;
    return SUCCESS;
   }
   return FALSE;
}

int FT_init(void){
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   if(bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oNRoot = NULL;
   ulCount = 0;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

int FT_destroy(void){
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   if(oNRoot) {
      ulCount -= Node_free(oNRoot);
      oNRoot = NULL;
   }

   bIsInitialized = FALSE;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

char *FT_toString(void);

/*----------------------------------------------------------------*/
static int FT_findNode(const char *pcPath, Node_T *poNResult, int nodeType) {
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;
   int iStatus;

   assert(pcPath != NULL);
   assert(poNResult != NULL);

   if(!bIsInitialized) {
      *poNResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poNResult = NULL;
      return iStatus;
   }

   iStatus = FT_traversePath(oPPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return iStatus;
   }

   if(oNFound == NULL) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   if(oNFound->type == nodeType && Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   Path_free(oPPath);
   *poNResult = oNFound;
   return SUCCESS;
}

static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest) {
   int iStatus;
   Path_T oPPrefix = NULL;
   Node_T oNCurr;
   Node_T oNChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;

   assert(oPPath != NULL);
   assert(poNFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oNRoot == NULL) {
      *poNFurthest = NULL;
      return SUCCESS;
   }

   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if(iStatus != SUCCESS) {
      *poNFurthest = NULL;
      return iStatus;
   }

   if(Path_comparePath(Node_getPath(oNRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oNCurr = oNRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poNFurthest = NULL;
         return iStatus;
      }
      if(Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = Node_getChild(oNCurr, ulChildID, &oNChild);
         if(iStatus != SUCCESS) {
            *poNFurthest = NULL;
            return iStatus;
         }
         oNCurr = oNChild;
      }
      else {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   Path_free(oPPrefix);
   *poNFurthest = oNCurr;
   return SUCCESS;
}