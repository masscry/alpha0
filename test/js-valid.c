#include <json2.h>
#include <stdio.h>

size_t screenPrintFunc(void* context, const void* buffer, size_t bufsize) {
  return fwrite(buffer, sizeof(char), bufsize, (FILE*)context);
}

int main(int argc, char* argv[]) {
  J2VAL j2root = 0;
  FILE* input = 0;

  if (argc != 2) {
    return -1;
  }

  input = fopen(argv[1], "r");
  if (input == 0) {
    return -1;
  }

  j2root = j2ParseFile(input);
  if (j2root == 0){
    return -1;
  }

  j2PrintFunc(screenPrintFunc, stdout, j2root, 0);
  fprintf(stdout, "\n");

  j2Cleanup(&j2root);
  fclose(input);
  return 0;
}
