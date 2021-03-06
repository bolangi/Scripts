#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <glob.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int StrEq(char* a, char* b) {
   return strcmp(a, b) == 0;
}
char* StrCat(char* a, char* b) {
   char* result = malloc(strlen(a) + strlen(b) + 1);
   strcpy(result, a);
   strcat(result, b);
   return result;
}
int IsDir(char* dir) {
   struct stat buf;
   int err = stat(dir, &buf);
   if (err != 0) return 0;
   if (S_ISDIR(buf.st_mode)) return 1;
   return 0;
}
char* BaseName(char* path) {
   char* result = malloc(strlen(path));
   char* base = strrchr(path, '/');
   if (!base)
      base = path;
   else
      base++;
   strcpy(result, base);
   return result;
}

char* full_path = NULL;
char* goboindex = NULL;

void LinkIfExists(char* dir) {
   char* path = StrCat(full_path, dir);
   if (IsDir(path)) {
      int i;
      glob_t globbuf;
      char* pattern = StrCat(path, "/*");
      glob(pattern, 0, NULL, &globbuf);
      for (i = 0; i < globbuf.gl_pathc; i++) {
         char* item = globbuf.gl_pathv[i];
         char* base = BaseName(item);
         unlink(base);
         errno = 0;
         if (symlink(item, base) < 0) {
            fprintf(stderr, "RescueSymlinkProgram: ");
            perror(base);
         }
         free(base);
      }
      free(pattern);
   }
   free(path);
}

int main(int argc, char** argv) {
   char* path;

   if (argc == 1 || StrEq(argv[1], "--help")) {
      fprintf(stderr, "RescueSymlinkProgram\n");
      fprintf(stderr, "A minimal functionality SymlinkProgram that can be used when the original script is not working.\n");
      fprintf(stderr, "Usage:   %s <program_path> [goboIndex]\n", argv[0]);
      fprintf(stderr, "Example: %s /Programs/Glibc/Current /System/Index\n", argv[0]);
      exit(1);
   }
   
   fprintf(stderr, "RescueSymlinkProgram: Warning, this script should only be used when SymlinkProgram fails.\n");

   if (IsDir(argv[1])) {
      full_path=argv[1];
      if (argc > 2 && IsDir(argv[2])) {
         goboindex=argv[2];
      } else {
         fprintf(stderr, "RescueSymlinkProgram: Assuming /System/Index as goboIndex directory.\n");
         goboindex="/System/Index";
      }
   } else {
      fprintf(stderr, "RescueSymlinkProgram: %s is not an existing directory.\n", argv[1]);
      exit(1);
   }
   
   if (!IsDir(goboindex)) {
      fprintf(stderr, "RescueSymlinkProgram: %s is not an existing directory.\n", goboindex);
      exit(1);
   }

   fprintf(stderr, "RescueSymlinkProgram: Linking %s inside %s\n", full_path, goboindex);

   path = StrCat(goboindex, "/bin");
   if (IsDir(path)) {
      assert(chdir(path) == 0);
      fprintf(stderr, "RescueSymlinkProgram: %s\n", path);
      LinkIfExists("/bin");
      LinkIfExists("/sbin");
      LinkIfExists("/Resources/Wrappers");
   }
   free(path);

   path = StrCat(goboindex, "/lib");
   if (IsDir(path)) {
      assert(chdir(path) == 0);
      fprintf(stderr, "RescueSymlinkProgram: %s\n", path);
      LinkIfExists("/lib");
   }
   free(path);
   
   path = StrCat(goboindex, "/libexec");
   if (IsDir(path)) {
      assert(chdir(path) == 0);
      fprintf(stderr, "RescueSymlinkProgram: %s\n", path);
      LinkIfExists("/libexec");
   }
   free(path);

   path = StrCat(goboindex, "/include");
   if (IsDir(path)) {
      assert(chdir(path) == 0);
      fprintf(stderr, "RescueSymlinkProgram: %s\n", path);
      LinkIfExists("/include");
   }
   free(path);

   path = StrCat(goboindex, "/../Settings");
   if (IsDir(path)) {
      assert(chdir(path) == 0);
      fprintf(stderr, "RescueSymlinkProgram: %s\n", path);
      /* The bash version added an extra readlink here to get rid
         of the .. in paths. We can live without this, can we? ;) */
      LinkIfExists("/../Settings");
   }
   free(path);

   /* Do not link Shared nor Manuals */

   fprintf(stderr, "RescueSymlinkProgram: Done\n");

   exit(0);
}
