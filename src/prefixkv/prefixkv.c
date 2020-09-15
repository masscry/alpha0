#ifdef __linux__
#define _GNU_SOURCE 
#endif

#include <stdlib.h>
#include <prefixkv.h>
#include "prfpriv.h"

void pkvCleanupBranch(pkvBranch_t branch) {
  if (branch->splt.level != 0) {
    if (branch->splt.child[0] != 0) {
      pkvCleanupBranch(branch->splt.child[0]);
      branch->splt.child[0] = 0;
    }
    if (branch->splt.child[1] != 0) {
      pkvCleanupBranch(branch->splt.child[1]);
      branch->splt.child[1] = 0;
    }
  } else {
    branch->leaf.data = 0;
  }
  free(branch);
}

void pkvCleanup(pkvTree_t tree) {
  if (tree != 0) {
    pkvCleanupBranch(tree->root);
    free(tree->data);
    free(tree);
  }
}

#ifdef _WIN32
void pkvSort(void* base, size_t total, size_t size, pkvCompareFunc_t compare, void* context)
{
  qsort_s(base, total, size, compare, context);
}
#endif

#ifdef __DOSX__

void* __DOSX_PKV_CONTEXT_PTR = NULL;

void pkvSort(void* base, size_t total, size_t size, pkvCompareFunc_t compare, void* context)
{
  stopswitch();
  __DOSX_PKV_CONTEXT_PTR = context;
  qsort(base, total, size, compare);
  strtswitch();
}
#endif

#ifdef __linux__

#include <stdlib.h>

void pkvSort(void* base, size_t total, size_t size, pkvCompareFunc_t compare, void* context)
{
  qsort_r(base, total, size, compare, context);
}

#endif /* __linux__ */
