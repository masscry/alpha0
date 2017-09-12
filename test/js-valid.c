#include <json2.h>
#include <stdio.h>

typedef struct fileContext {
  FILE* file;
  int peeked;
} fileContext;

int fileGetCharFunc(void* context){
  fileContext* fc = (fileContext*)context;
  int smb = fgetc(fc->file);
  fc->peeked = smb;
  return fc->peeked;
}

int filePeekCharFunc(void* context){
  fileContext* fc = (fileContext*)context;
  if (fc->peeked < 0) {
    fc->peeked = fileGetCharFunc(context);
  }
  return fc->peeked;
}

size_t screenPrintFunc(void* context, const void* buffer, size_t bufsize) {
  return fwrite(buffer, sizeof(char), bufsize, (FILE*)context);
}

int main(int argc, char* argv[]) {
  fileContext context;
  j2ParseCallback calls;
  J2VAL j2root = 0;
  FILE* input = 0;

  if (argc != 2) {
    return -1;
  }

  input = fopen(argv[1], "r");
  if (input == 0) {
    return -1;
  }

  context.peeked = -1;
  context.file = input;

  calls.get = fileGetCharFunc;
  calls.peek = filePeekCharFunc;

  j2root = j2ParseFunc(calls, &context);
  if (j2root == 0){
    return -1;
  }

  j2PrintFunc(screenPrintFunc, stdout, j2root, 0);
  fprintf(stdout, "\n");

  j2Cleanup(&j2root);
  fclose(input);
  return 0;
}
