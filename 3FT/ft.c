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
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc);
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc);

int FT_insertDir(const char *pcPath)
{
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   if (!bIsInitialized)
      return INITIALIZATION_ERROR;

   /* validate pcPath and generate a Path_T for it */
   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus = FT_traversePath(oPPath, &oNCurr);
   if (iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if (oNCurr == NULL && oNRoot != NULL)
   {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   if (oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else if(Node_getType(oNCurr) == 1){
      Path_free(oPPath);
      return NOT_A_DIRECTORY;
   }
   else
   {
      ulIndex = Path_getDepth(Node_getPath(oNCurr)) + 1;

      /* oNCurr is the node we're trying to insert */
      if (ulIndex == ulDepth + 1 && !Path_comparePath(oPPath,
                                                      Node_getPath(oNCurr)))
      {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while (ulIndex <= ulDepth)
   {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if (iStatus != SUCCESS)
      {
         Path_free(oPPath);
         if (oNFirstNew != NULL)
            (void)Node_free(oNFirstNew);
         /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 0, NULL, 0); /* NEHA DEBUG: changed last NULL to 0 */
      if (iStatus != SUCCESS)
      {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if (oNFirstNew != NULL)
            (void)Node_free(oNFirstNew);
         /*we dont need to use any checker right*/
         /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if (oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if (oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

boolean FT_containsDir(const char *pcPath)
{
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findNode(pcPath, &oNFound, 0);
   return (boolean)(iStatus == SUCCESS);
}

int FT_rmDir(const char *pcPath)
{
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount))*/

   iStatus = FT_findNode(pcPath, &oNFound, 0);

   if (iStatus != SUCCESS)
      return iStatus;

   ulCount -= Node_free(oNFound);
   if (ulCount == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength)
{
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
   if (!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus = FT_traversePath(oPPath, &oNCurr);
   if (iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /*file cannot be a root*/
   if (oNCurr == NULL)
   {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   ulIndex = Path_getDepth(Node_getPath(oNCurr)) + 1;

   /* oNCurr is the node we're trying to insert */
   if (ulIndex == ulDepth + 1 && !Path_comparePath(oPPath, Node_getPath(oNCurr)))
   {
      Path_free(oPPath);
      return ALREADY_IN_TREE;
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while (ulIndex <= ulDepth)
   {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if (iStatus != SUCCESS)
      {
         Path_free(oPPath);
         if (oNFirstNew != NULL)
            (void)Node_free(oNFirstNew);
         /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* insert the new node for this level */
      /*new node looks a bit different here right? and dont change dir*/
      if (ulIndex == ulDepth)
         iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 1, pvContents, ulLength);
      else
         iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, 0, NULL, 0);

      if (iStatus != SUCCESS)
      {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if (oNFirstNew != NULL)
            (void)Node_free(oNFirstNew);
         /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if (oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if (oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
   return SUCCESS;
}

boolean FT_containsFile(const char *pcPath)
{
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findNode(pcPath, &oNFound, 1);
   return (boolean)(iStatus == SUCCESS);
}

int FT_rmFile(const char *pcPath)
{
   int iStatus;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount))*/

   iStatus = FT_findNode(pcPath, &oNFound, 1);

   if (iStatus != SUCCESS)
      return iStatus;

   ulCount -= Node_free(oNFound);
   if (ulCount == 0)
      oNRoot = NULL;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

void *FT_getFileContents(const char *pcPath)
{
   Node_T oNFound = NULL;
   int iStatus;

   Path_T input;
   if (!Path_new((const char *)pcPath, &input))
   {
      return FALSE;
   }

   iStatus = FT_traversePath(input, &oNFound);
   if (iStatus != SUCCESS)
   {
      return FALSE;
   }
   if (Node_getType(oNFound) == 1)
      return Node_getContents(oNFound);
   return FALSE;
}

/*ASK ASK ASK about if you have to anything werid free or just pointer too*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength)
{
   Node_T oNFound = NULL;
   void *oldContent = NULL;

   int iStatus;

   Path_T input;
   if (!Path_new((const char *)pcPath, &input))
   {
      return FALSE;
   }

   iStatus = FT_traversePath(input, &oNFound);
   if (iStatus != SUCCESS)
   {
      return NULL;
   }
   if (Node_getType(oNFound) == 1)
   {
      oldContent = Node_getContents(oNFound);
      Node_setContents(oNFound, pvNewContents);
      Node_setContentSize(oNFound, ulNewLength);
      return oldContent;
   }
   return NULL;
}

/*go through this and previous make sure it test for bugs as specified by the
.h file */
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize)
{
   Node_T oNFound = NULL;
   int iStatus;

   Path_T input;
   if (!Path_new((const char *)pcPath, &input))
   {
      return FALSE;
   }

   iStatus = FT_traversePath(input, &oNFound);
   if (iStatus != SUCCESS)
   {
      return iStatus;
   }
   if (Node_getType(oNFound) == 0)
   {
      pbIsFile = FALSE;
      return SUCCESS;
   }
   else if (Node_getType(oNFound) == 1)
   {
      pbIsFile = TRUE;
      pulSize = Node_getContentSize(oNFound);
      return SUCCESS;
   }
   return FALSE;
}

int FT_init(void)
{
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   if (bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oNRoot = NULL;
   ulCount = 0;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

int FT_destroy(void)
{
   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/

   if (!bIsInitialized)
      return INITIALIZATION_ERROR;

   if (oNRoot)
   {
      ulCount -= Node_free(oNRoot);
      oNRoot = NULL;
   }

   bIsInitialized = FALSE;

   /*assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));*/
   return SUCCESS;
}

/*----------------------------------------------------------------*/
static int FT_findNode(const char *pcPath, Node_T *poNResult, int nodeType)
{
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;
   int iStatus;

   assert(pcPath != NULL);
   assert(poNResult != NULL);

   if (!bIsInitialized)
   {
      *poNResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if (iStatus != SUCCESS)
   {
      *poNResult = NULL;
      return iStatus;
   }

   iStatus = FT_traversePath(oPPath, &oNFound);
   if (iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return iStatus;
   }

   if (oNFound == NULL)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }

   if (Node_getType(oNFound) == nodeType && Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   /*CHECK CHECK CHECK*/
   if (Node_getType(oNFound) != nodeType && Path_comparePath(Node_getPath(oNFound), oPPath) == 0)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return CONFLICTING_PATH;
   }

   Path_free(oPPath);
   *poNResult = oNFound;
   return SUCCESS;
}

static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest)
{
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
   if (oNRoot == NULL)
   {
      *poNFurthest = NULL;
      return SUCCESS;
   }

   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if (iStatus != SUCCESS)
   {
      *poNFurthest = NULL;
      return iStatus;
   }

   if (Path_comparePath(Node_getPath(oNRoot), oPPrefix))
   {
      Path_free(oPPrefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oNCurr = oNRoot;
   ulDepth = Path_getDepth(oPPath);
   for (i = 2; i <= ulDepth; i++)
   {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if (iStatus != SUCCESS)
      {
         *poNFurthest = NULL;
         return iStatus;
      }
      if (Node_hasChild(oNCurr, oPPrefix, &ulChildID))
      {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = Node_getChild(oNCurr, ulChildID, &oNChild);
         if (iStatus != SUCCESS)
         {
            *poNFurthest = NULL;
            return iStatus;
         }
         oNCurr = oNChild;
      }
      else
      {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   Path_free(oPPrefix);
   *poNFurthest = oNCurr;
   return SUCCESS;
}

/*--------------------------------------------------------------------------------------------*/
/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i)
{
   size_t c;

   assert(d != NULL);

   if (n != NULL)
   {
      (void)DynArray_set(d, i, n);
      i++;
      for (c = 0; c < Node_getNumChildren(n); c++)
      {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getChild(n, c, &oNChild);
         assert(iStatus == SUCCESS);
         if (Node_getType(oNChild) == 1)
         {
            DynArray_set(d, i, oNChild);
            i++;
         }
      }
      for (c = 0; c < Node_getNumChildren(n); c++)
      {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getChild(n, c, &oNChild);
         assert(iStatus == SUCCESS);
         if (Node_getType(oNChild) == 0)
            i = FT_preOrderTraversal(oNChild, d, i);
      }
   }
   return i;
}

char *FT_toString(void)
{
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if (!bIsInitialized)
      return NULL;

   nodes = DynArray_new(ulCount);
   (void)FT_preOrderTraversal(oNRoot, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void *))FT_strlenAccumulate,
                (void *)&totalStrlen);

   result = malloc(totalStrlen);
   if (result == NULL)
   {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void *))FT_strcatAccumulate,
                (void *)result);

   DynArray_free(nodes);

   return result;
}

/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc)
{
   assert(pulAcc != NULL);

   if (oNNode != NULL)
      *pulAcc += (Path_getStrLength(Node_getPath(oNNode)) + 1);
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc)
{
   assert(pcAcc != NULL);

   if (oNNode != NULL)
   {
      strcat(pcAcc, Path_getPathname(Node_getPath(oNNode)));
      strcat(pcAcc, "\n");
   }
}

/*----------------------------------------------------------------*/